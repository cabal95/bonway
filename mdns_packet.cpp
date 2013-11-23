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
    QueryVector::iterator	qit;
    map<string, int>		names;
    DataBuffer			data(1500);
    size_t			lastSafeSize = 0;
    int				qCount = 0, lastQCount, aCount = 0, lastACount;


    if (queries->size() == 0)
	return DataBuffer(0);

    data.putInt16(0);
    data.putInt16(0); // flags, 0 == query
    data.putInt16(0); // Query count.
    data.putInt16(0); // Answer count.
    data.putInt16(0); // Nameserver count.
    data.putInt16(0); // Additionals count.
    lastSafeSize = data.getSize();
    lastQCount = qCount;
    lastACount = aCount;

    for (qit = queries->begin(); qit != queries->end(); ) {
	query *q = *qit;

	q->encode(data, &names);
	qCount += 1;

	if (q->getType() == RR_TYPE_PTR) {
	    //
	    // Check all records for type PTR whose name matches the
	    // service_name.
	    //
	}

	if (data.getSize() <= 1500) {
	    lastSafeSize = data.getSize();
	    delete q;
	    qit = queries->erase(qit);
	}
	else {
	    data.setSize(lastSafeSize);
	    qCount = lastQCount;
	    aCount = lastACount;

	    break;
	}
    }

    //
    // Update the query count and answer count.
    //
    data.seek(4, SEEK_SET);
    data.putInt16(htons(qCount));
    data.putInt16(htons(aCount));
    data.seek(0, SEEK_END);

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
    size_t			lastSafeSize = 0;
    int				rrCount = 0, lastRRCount, adCount = 0, lastADCount;


    if (answers->size() == 0)
	return DataBuffer(0);

    data.putInt16(0);
    data.putInt16(htons(MDNS_PACKET_FLAG_AN | MDNS_PACKET_FLAG_AA)); // flags
    data.putInt16(0); // Query count.
    data.putInt16(0); // Answer count.
    data.putInt16(0); // Nameserver count.
    data.putInt16(0); // Additionals count.
    lastSafeSize = data.getSize();
    lastRRCount = rrCount;
    lastADCount = adCount;

    for (rrit = answers->begin(); rrit != answers->end(); ) {
	record *rr = *rrit;

	rr->encode(data, &names);
	rrCount += 1;

	if (rr->getType() == RR_TYPE_PTR) {
	    //
	    // Check all records for type PTR whose name matches the
	    // service_name and add them to the additionals.
	    //
	}

	if (data.getSize() <= 1500) {
	    lastSafeSize = data.getSize();
	    delete rr;
	    rrit = answers->erase(rrit);
	}
	else {
	    data.setSize(lastSafeSize);
	    rrCount = lastRRCount;
	    adCount = lastADCount;

	    break;
	}
    }

    //
    // Update the answer count and additionals count.
    //
    data.seek(6, SEEK_SET);
    data.putInt16(htons(rrCount));
    data.seek(10, SEEK_SET);
    data.putInt16(htons(adCount));
    data.seek(0, SEEK_END);

    return data;
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

