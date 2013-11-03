#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <iostream>
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

