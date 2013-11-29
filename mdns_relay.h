#ifndef __MDNS_RELAY_H__
#define __MDNS_RELAY_H__

//#include "mdns_socket.h"
//#include "mdns_list.h"
//#include "mdns_util.h"
#include <map>
#include "mdns_packet.h"
#include "mdns_query.h"
#include "mdns_record.h"
#include "mdns_socket.h"
#include "mdns_a_record.h"
#include "types.h"


namespace mDNS {

class RelayService
{
protected:
    std::vector<std::string>	m_types;
    std::vector<int>		m_clients, m_servers;

public:
    RelayService();
    RelayService(std::string type);
    RelayService(std::string type, int client, int server);

    void addType(std::string type);
    void addClientInterface(int interface);
    void addServerInterface(int interface);

    friend class Relay;
};


class Relay
{
private:
    std::map<int, QueryVector>	m_query_queue;
    std::map<int, RecordVector>	m_known_records, m_answer_queue;
    mtime_t			m_last_expire_check;
    std::vector<RelayService>	m_services;

protected:
    void sendQueries(Socket &socket);
    void sendAnswers(Socket &socket);

    const RelayService *isQueryAllowed(std::string type, int interface) const;
    const RelayService *isAnswerAllowed(std::string type, int interface) const;

    void sendKnownServiceTypes(int interface);

    void processQueryPacket(const packet *pkt, int interface);
    void processAnswerPacket(const packet *pkt, int interface);
    void relayServiceQuery(const RelayService *service, const query *q);
    void relayAQuery(const query *q, int interface);
    void relayServiceAnswer(const RelayService *service, const record *rr, int interface);
    void relayAAnswer(const a_record *a, int interface);
    void relayAServiceAnswer(const a_record *a, int interface, string service_name, std::list<int> &ifaces);

    void checkCacheExpiry();

public:
    Relay();
    ~Relay();

    void addService(RelayService service);

    int processPacket(Socket &socket, const packet *pkt, struct sockaddr from, int interface);

    void terminate(Socket &socket);
};

} /* namespace mDNS */

#endif /* __MDNS_RELAY_H__ */

