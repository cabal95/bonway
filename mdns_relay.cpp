#include <algorithm>
#include <iostream>
#include "mdns.h"
#include "mdns_relay.h"
#include "mdns_a_record.h"
#include "mdns_ptr_record.h"
#include "mdns_srv_record.h"
#include "mdns_util.h"


using namespace std;
namespace mDNS {


RelayService::RelayService()
{
}


RelayService::RelayService(string type)
{
    m_types.push_back(type);
}


RelayService::RelayService(string type, int client, int server)
{
    m_types.push_back(type);
    m_clients.push_back(client);
    m_servers.push_back(server);
}


Relay::Relay()
{
    m_last_expire_check = 0;
}


Relay::~Relay()
{
    map<int, QueryVector>::iterator	mqit;
    map<int, RecordVector>::iterator	mrit;
    QueryVector::iterator		qit;
    RecordVector::iterator		rit;


    for (mqit = m_query_queue.begin(); mqit != m_query_queue.end(); mqit++) {
	QueryVector &qv = mqit->second;

	for (qit = qv.begin(); qit != qv.end(); qit++)
	    delete *qit;
    }

    for (mrit = m_known_records.begin(); mrit != m_known_records.end(); mrit++) {
	RecordVector &rv = mrit->second;

	for (rit = rv.begin(); rit != rv.end(); rit++)
	    delete *rit;
    }

    for (mrit = m_answer_queue.begin(); mrit != m_answer_queue.end(); mrit++) {
	RecordVector &rv = mrit->second;

	for (rit = rv.begin(); rit != rv.end(); rit++)
	    delete *rit;
    }
}


void Relay::addService(RelayService service)
{
    vector<string>::const_iterator	sit;
    vector<int>::const_iterator		iit;


    m_services.push_back(service);

    //
    // Send an initial query for this service.
    //
    for (sit = service.m_types.begin(); sit != service.m_types.end(); sit++) {
	query	q = query(*sit + ".local", RR_TYPE_PTR, RR_CLASS_IN);
	for (iit = service.m_servers.begin(); iit != service.m_servers.end(); iit++) {
	    m_query_queue[*iit].push_back(new query(q));
	}
    }
}


int Relay::processPacket(Socket &socket, const packet *pkt, struct sockaddr from, int interface)
{
    if (pkt->flags() & MDNS_PACKET_FLAG_AN) {
	processAnswerPacket(pkt, interface);
    }
    else {
	processQueryPacket(pkt, interface);
    }

    checkCacheExpiry();
    sendQueries(socket);
    sendAnswers(socket);

    return 0;
}



void Relay::sendQueries(Socket &socket)
{
    map<int, QueryVector>::iterator	mqit;
    QueryVector::iterator		qit;
    DataBuffer				buffer;


    for (mqit = m_query_queue.begin(); mqit != m_query_queue.end(); mqit++) {
	QueryVector &qv = mqit->second;

	while (qv.size() > 0) {
	    buffer = packet::serializeQueries(&qv, &m_known_records[mqit->first]);
	    socket.send(buffer, mqit->first);
	}
    }
}


void Relay::sendAnswers(Socket &socket)
{
    map<int, RecordVector>::iterator	mrit;
    RecordVector::iterator		rit;
    DataBuffer				buffer;


    for (mrit = m_answer_queue.begin(); mrit != m_answer_queue.end(); mrit++) {
	RecordVector &rv = mrit->second;

	while (rv.size() > 0) {
	    buffer = packet::serializeAnswers(&rv, &m_known_records[mrit->first]);
	    socket.send(buffer, mrit->first);
	}
    }
}


const RelayService *Relay::isQueryAllowed(string type, int interface) const
{
    vector<RelayService>::const_iterator	rsit;
    vector<string>::const_iterator		sit;
    vector<int>::const_iterator			iit;


    for (rsit = m_services.begin(); rsit != m_services.end(); rsit++) {
	//
	// Queries come in on the client interface.
	//
	for (iit = (*rsit).m_clients.begin(); iit != (*rsit).m_clients.end(); iit++) {
	    if (*iit == interface)
		break;
	}
	if (iit == (*rsit).m_clients.end())
	    continue;

	//
	// Match the name.
	//
	for (sit = (*rsit).m_types.begin(); sit != (*rsit).m_types.end(); sit++) {
	    if (type == *sit)
		return &(*rsit);
	}
    }

    return NULL;
}


const RelayService *Relay::isAnswerAllowed(string type, int interface) const
{
    vector<RelayService>::const_iterator	rsit;
    vector<string>::const_iterator		sit;
    vector<int>::const_iterator			iit;


    for (rsit = m_services.begin(); rsit != m_services.end(); rsit++) {
	//
	// Answers come in on the server interface.
	//
	for (iit = (*rsit).m_servers.begin(); iit != (*rsit).m_servers.end(); iit++) {
	    if (*iit == interface)
		break;
	}
	if (iit == (*rsit).m_servers.end())
	    continue;

	//
	// Match the name.
	//
	for (sit = (*rsit).m_types.begin(); sit != (*rsit).m_types.end(); sit++) {
	    if (type == *sit)
		return &(*rsit);
	}
    }

    return NULL;
}


//
// Send the list of known service types back to the specified interface.
// This is built from the configuration information.
//
void Relay::sendKnownServiceTypes(int interface)
{
    vector<RelayService>::const_iterator	rsit;
    vector<string>::const_iterator		sit;
    vector<int>::const_iterator			iit;
    vector<string>				types;
    ptr_record					*ptr;


    for (rsit = m_services.begin(); rsit != m_services.end(); rsit++) {
	//
	// We need to send the service type answer to the client interface.
	//
	for (iit = (*rsit).m_clients.begin(); iit != (*rsit).m_clients.end(); iit++) {
	    if (*iit == interface)
		break;
	}
	if (iit == (*rsit).m_clients.end())
	    continue;

	//
	// Check each type to see if it needs to be added to the list.
	//
	for (sit = (*rsit).m_types.begin(); sit != (*rsit).m_types.end(); sit++) {
	    if (std::find(types.begin(), types.end(), *sit) == types.end())
		types.push_back(*sit);
	}
    }

    //
    // We have a list of known service types, put them in the answer queue.
    //
    for (sit = types.begin(); sit != types.end(); sit++) {
	ptr = new ptr_record("_services._dns-sd._udp.local", RR_CLASS_IN, 3600,
                             (*sit) + ".local");
	m_answer_queue[interface].push_back(ptr);
    }
}


void Relay::processQueryPacket(const packet *pkt, int interface)
{
    QueryVector::const_iterator	qvit;
    const RelayService		*rs;
    string			svcname;
    query			*q;


    for (qvit = pkt->queries().begin(); qvit != pkt->queries().end(); qvit++) {
	q = *qvit;

	if (q->isService()) {
	    svcname = q->getServiceName();

	    if (svcname == "_services._dns-sd._udp") {
		sendKnownServiceTypes(interface);
	    }
	    else {
		rs = isQueryAllowed(svcname, interface);
		if (rs != NULL)
		    relayServiceQuery(rs, q);
	    }
	}
	else if (q->getNameSegments().size() == 2 && q->getType() == RR_TYPE_A)
	    relayAQuery(q, interface);
    }
}


void Relay::processAnswerPacket(const packet *pkt, int interface)
{
    RecordVector::const_iterator	rvit;
    const RelayService			*rs;
    string				svcname;
    record				*rr;


    for (rvit = pkt->answers().begin(); rvit != pkt->answers().end(); rvit++) {
	rr = *rvit;

	if (rr->isService()) {
	    rs = isAnswerAllowed(rr->getServiceName(), interface);
	    if (rs != NULL)
		relayServiceAnswer(rs, rr, interface);
	}
	else if (rr->getNameSegments().size() == 2 && rr->getType() == RR_TYPE_A)
	    relayAAnswer((a_record *)rr, interface);
    }
}


void Relay::relayServiceQuery(const RelayService *rs, const query *q)
{
    vector<int>::const_iterator	iit;


    for (iit = rs->m_servers.begin(); iit != rs->m_servers.end(); iit++) {
	m_query_queue[*iit].push_back(new query(*q));
    }
}


void Relay::relayServiceAnswer(const RelayService *rs, const record *rr, int interface)
{
    vector<int>::const_iterator		iit;
    RecordVector::iterator		rvit;


    for (iit = rs->m_clients.begin(); iit != rs->m_clients.end(); iit++) {
	m_answer_queue[*iit].push_back(rr->clone());
    }

    for (rvit = m_known_records[interface].begin(); rvit != m_known_records[interface].end(); rvit++) {
	if (rr->isSame(*rvit) == true) {
	    delete *rvit;
	    m_known_records[interface].erase(rvit);

	    break;
	}
    }

    m_known_records[interface].push_back(rr->clone());
}


void Relay::checkCacheExpiry()
{
    map<int, RecordVector>::iterator	mrit;
    RecordVector::iterator		rrit;
    time_t				now = time(NULL);


    if ((m_last_expire_check + 5000) > util::time())
	return;

    for (mrit = m_known_records.begin(); mrit != m_known_records.end(); mrit++) {
	for (rrit = mrit->second.begin(); rrit != mrit->second.end(); ) {
	    if (((*rrit)->getTTLBase() + (*rrit)->getTTL()) < now) {
		cout << "Expiring record " << (*rrit)->getName() << endl;
		delete *rrit;
		rrit = mrit->second.erase(rrit);
	    }
	    else
		rrit++;
	}
    }

    m_last_expire_check = util::time();
}


void Relay::relayAQuery(const query *q, int interface)
{
    map<int, RecordVector>::iterator	mrit;
    RecordVector::iterator		rrit;
    srv_record				*srv;


    for (mrit = m_known_records.begin(); mrit != m_known_records.end(); mrit++) {
	if (mrit->first == interface)
	    continue;

	for (rrit = mrit->second.begin(); rrit != mrit->second.end(); rrit++) {
	    if ((*rrit)->getType() == RR_TYPE_SRV) {
		srv = (srv_record *)(*rrit);
		if (srv->getTargetName() == q->getName()) {
		    m_query_queue[mrit->first].push_back(new query(*q));
		    break;
		}
	    }
	}
    }
}


void Relay::relayAAnswer(const a_record *a, int interface)
{
    RecordVector::iterator	rrit;
    srv_record			*srv;
    list<int>			ifaces;


    //
    // Check if this is an A record for a known service.
    //
    for (rrit = m_known_records[interface].begin(); rrit != m_known_records[interface].end(); rrit++) {
	if ((*rrit)->getType() == RR_TYPE_SRV) {
	    srv = (srv_record *)(*rrit);
	    if (srv->getTargetName() == a->getName())
		relayAServiceAnswer(a, interface, srv->getServiceName(), ifaces);
	}
    }
}


void Relay::relayAServiceAnswer(const a_record *a, int interface, string service_name, list<int> &ifaces)
{
    vector<int>::const_iterator	iit;
    const RelayService		*rs;


    rs = isAnswerAllowed(service_name, interface);
    if (rs == NULL)
	return;

    for (iit = rs->m_clients.begin(); iit != rs->m_clients.end(); iit++) {
	if (find(ifaces.begin(), ifaces.end(), *iit) != ifaces.end())
	    continue;

	ifaces.push_back(*iit);
	m_answer_queue[*iit].push_back(a->clone());
    }
}


//
// Perform a clean termination. Mark all known answers as expired and
// send them out.
//
void Relay::terminate(Socket &socket)
{
    map<int, RecordVector>::iterator	mrit;
    RecordVector::iterator		rvit;
    vector<int>::const_iterator		iit;


    for (mrit = m_known_records.begin(); mrit != m_known_records.end(); mrit++) {
	RecordVector &rv = mrit->second;

	for (rvit = rv.begin(); rvit != rv.end(); ) {
	    record	*rr = *rvit;

	    if (rr->isService()) {
		const RelayService	*rs;

		rs = isAnswerAllowed(rr->getServiceName(), mrit->first);
		if (rs != NULL) {
cout << "Sending expire for " << rr->getName() << endl;
		    rr->setTTL(0);
		    for (iit = rs->m_clients.begin(); iit != rs->m_clients.end(); iit++) {
			m_answer_queue[*iit].push_back(rr->clone());
		    }
		}
	    }

	    //
	    // Cleanup as we go, no associated records should be sent.
	    //
	    delete rr;
	    rvit = rv.erase(rvit);
	}
    }

    sendAnswers(socket);
}


} /* namespace mDNS */

