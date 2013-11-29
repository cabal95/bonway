#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <iostream>
#include "mdns.h"
#include "mdns_query.h"
#include "mdns_record.h"
#include "mdns_ptr_record.h"
#include "mdns_srv_record.h"
#include "databuffer.h"
#include "mdns_packet.h"


using namespace std;
namespace mDNS {


packet::packet()
{
    m_flags = 0;
}


packet::~packet()
{
    QueryVector::iterator	qvit;
    RecordVector::iterator	rvit;


    for (qvit = m_queries.begin(); qvit != m_queries.end(); qvit++) {
	delete *qvit;
    }

    for (rvit = m_answers.begin(); rvit != m_answers.end(); rvit++) {
	delete *rvit;
    }

    for (rvit = m_nameservers.begin(); rvit != m_nameservers.end(); rvit++) {
	delete *rvit;
    }

    for (rvit = m_additionals.begin(); rvit != m_additionals.end(); rvit++) {
	delete *rvit;
    }
}


packet *packet::deserialize(DataBuffer &data)
{
    uint16_t	qdcount, ancount, nscount, arcount;
    packet	*pkt = NULL;
    int		i;


    if (data.getAvailable() < (sizeof(uint16_t) * 6))
        return NULL;

    pkt = new packet();
    data.readInt16();
    pkt->m_flags = ntohs(data.readInt16());
    qdcount = ntohs(data.readInt16());
    ancount = ntohs(data.readInt16());
    nscount = ntohs(data.readInt16());
    arcount = ntohs(data.readInt16());

    //
    // Read all the queries.
    //
    for (i = 0; i < qdcount; i++) {
	query *q = query::decode(data);
	pkt->m_queries.push_back(q);
    }

    //
    // Read all the answers.
    //
    for (i = 0; i < ancount; i++) {
	record *rr = record::decode(data);
	if (rr != NULL)
	    pkt->m_answers.push_back(rr);
    }

    //
    // Read all the nameservers.
    //
    for (i = 0; i < nscount; i++) {
	record *rr = record::decode(data);
	if (rr != NULL)
	    pkt->m_nameservers.push_back(rr);
    }

    //
    // Read all the additional records.
    //
    for (i = 0; i < arcount; i++) {
	record *rr = record::decode(data);
	if (rr != NULL)
	    pkt->m_additionals.push_back(rr);
    }

    return pkt;
}


DataBuffer packet::serialize()
{
    RecordVector::iterator	rriter;
    QueryVector::iterator	qiter;
    map<string, int>		names;
    DataBuffer			data(1500);


    data.putInt16(0);
    data.putInt16(htons(m_flags));
    data.putInt16(htons(m_queries.size()));
    data.putInt16(htons(m_answers.size()));
    data.putInt16(htons(m_nameservers.size()));
    data.putInt16(htons(m_additionals.size()));

    for (qiter = m_queries.begin(); qiter != m_queries.end(); qiter++) {
	query	*q = *qiter;
	q->encode(data, &names);
    }

    for (rriter = m_answers.begin(); rriter != m_answers.end(); rriter++) {
	record	*rr = *rriter;
	rr->encode(data, &names);
    }

    for (rriter = m_nameservers.begin(); rriter != m_nameservers.end(); rriter++) {
	record	*rr = *rriter;
	rr->encode(data, &names);
    }

    for (rriter = m_additionals.begin(); rriter != m_additionals.end(); rriter++) {
	record	*rr = *rriter;
	rr->encode(data, &names);
    }

    return data;
}


//
// Build a temporary packet will contain the given queries and serialize
// the data. The packet is built one query at a time to ensure that we do
// not go over the max packet size. When a query is added the answers vector
// is checked to see if any known answers should be included as well.
// Each time a query is added to the packet successfully it is removed from
// the queries vector.
//
// If the query is a for PTR records then check the records vector for any
// records that have the same name as the PTR record add to the answers
// section.
//
DataBuffer packet::serializeQueries(QueryVector *queries, RecordVector *records)
{
    RecordVector::iterator	rrit;
    QueryVector::iterator	qit;
    map<string, int>		names;
    DataBuffer			data(1500);
    packet			*pkt;


    if (queries->size() == 0)
	return DataBuffer(0);
    pkt = new packet();
    pkt->flags(0);

    data.putInt16(0);
    data.putInt16(0); // flags.
    data.putInt16(0); // Query count.
    data.putInt16(0); // Answer count.
    data.putInt16(0); // Nameserver count.
    data.putInt16(0); // Additionals count.

    for (qit = queries->begin(); qit != queries->end(); ) {
	query *q = *qit;

	q->encode(data, &names);

#if 0
// TODO: Right now this doesn't work. We need a way to determine which
// interface the packet came from.
	//
	// Add any known answers already.
	//
	if (q->getType() == RR_TYPE_PTR) {
	    //
	    // Check all records whose name matches the service_name.
	    //
	    for (rrit = records->begin(); rrit != records->end(); rrit++) {
		if (q->getName() == (*rrit)->getName()) {
		    (*rrit)->encode(data, &names);

		    if (data.getSize() <= 1500) {
			pkt->addAnswer((*rrit)->clone());
		    }
		    else {
			break;
		    }
		}
	    }
	}
#endif

	if (data.getSize() <= 1500) {
	    pkt->addQuery(q);
	    qit = queries->erase(qit);
	}
	else {
	    break;
	}
    }

    data = pkt->serialize();
    delete pkt;

    return data;
}


//
// Build a databuffer that contains the answers in the passed vector. If
// the databuffer runs out of room then no more answers will be added to
// the buffer and it will be returned.
//
// When an answer is added to the buffer any additional records are added
// as well. Each answer added is removed from the answers vector to indicate
// that it has been handled.
//
DataBuffer packet::serializeAnswers(RecordVector *answers, RecordVector *records)
{
    RecordVector::iterator	rrit;
    map<string, int>		names;
    DataBuffer			data(1500);
    packet			*pkt;


    if (answers->size() == 0)
	return DataBuffer(0);
    pkt = new packet();
    pkt->flags(MDNS_PACKET_FLAG_AN | MDNS_PACKET_FLAG_AA);

    data.putInt16(0);
    data.putInt16(0); // flags.
    data.putInt16(0); // Query count.
    data.putInt16(0); // Answer count.
    data.putInt16(0); // Nameserver count.
    data.putInt16(0); // Additionals count.

    for (rrit = answers->begin(); rrit != answers->end(); ) {
	record *rr = *rrit;

	rr->encode(data, &names);

#if 0
// TODO: See above
	if (rr->getType() == RR_TYPE_PTR) {
	    ptr_record *ptr = (ptr_record *)rr;

cout << "Looking for additional records for " << ptr->getTargetName() << ".\r\n";
	    serializeAddMatchingRecords(ptr->getTargetName(), data, pkt,
					&names, records);
	}
#endif

	if (data.getSize() <= 1500) {
	    pkt->addAnswer(rr);
	    rrit = answers->erase(rrit);
	}
	else {
	    break;
	}
    }

    data = pkt->serialize();
    delete pkt;

    return data;
}


bool packet::serializeAddMatchingRecords(string name, DataBuffer &data,
					 packet *pkt, map<string, int> *names,
					 RecordVector *records)
{
    RecordVector::iterator	rvit;
    RecordVector		toAdd;
    record			*rr;


    for (rvit = records->begin(); rvit != records->end(); rvit++) {
	rr = *rvit;

cout << "  Checking record " << rr->getName() << ".\r\n";
	if (name == rr->getName()) {
cout << "    Found record type " << rr->getType() << ".\r\n";
	    rr->encode(data, names);

	    //
	    // This is all or nothing. We cannot add a primary record without
	    // being able to add all the secondaries as well.
	    //
	    if (data.getSize() > 1500)
		return false;

	    //
	    // If this is a SRV record then chain, for example to pickup any
	    // A or AAAA records.
	    //
	    if (rr->getType() == RR_TYPE_SRV) {
		srv_record *srv = (srv_record *)rr;

		if (serializeAddMatchingRecords(srv->getTargetName(), data,
						pkt, names, records) == false) {
		    return false;
		}
	    }

	    toAdd.push_back(rr);
	}
    }

    //
    // Add all the records that matched this name.
    //
    for (rvit = toAdd.begin(); rvit != toAdd.end(); rvit++) {
	pkt->addAdditional((*rvit)->clone());
    }

    return true;
}


uint16_t packet::flags() const
{
    return m_flags;
}


void packet::flags(uint16_t flags)
{
    m_flags = flags;
}


bool packet::isQuery() const
{
    return ((m_flags & MDNS_PACKET_FLAG_AN) == 0);
}


const QueryVector &packet::queries() const
{
    return m_queries;
}


void packet::addQuery(query *q)
{
    m_queries.push_back(q);
}


const RecordVector &packet::answers() const
{
    return m_answers;
}


void packet::addAnswer(record *rr)
{
    m_answers.push_back(rr);
}


const RecordVector &packet::nameservers() const
{
    return m_nameservers;
}


void packet::addNameserver(record *rr)
{
    m_nameservers.push_back(rr);
}


const RecordVector &packet::additionals() const
{
    return m_additionals;
}


void packet::addAdditional(record *rr)
{
    m_additionals.push_back(rr);
}


void packet::dump()
{
    RecordVector::iterator	rriter;
    QueryVector::iterator	qiter;


    for (qiter = m_queries.begin(); qiter != m_queries.end(); qiter++) {
	query	*q = *qiter;
	cout << "QD: " << q->toString() << endl;
    }

    for (rriter = m_answers.begin(); rriter != m_answers.end(); rriter++) {
	record	*rr = *rriter;
	cout << "AN: " << rr->toString() << endl;
    }

    for (rriter = m_nameservers.begin(); rriter != m_nameservers.end(); rriter++) {
	record	*rr = *rriter;
	cout << "NS: " << rr->toString() << endl;
    }

    for (rriter = m_additionals.begin(); rriter != m_additionals.end(); rriter++) {
	record	*rr = *rriter;
	cout << "AR: " << rr->toString() << endl;
    }

    cout << endl;
}


} /* namespace mDNS */

