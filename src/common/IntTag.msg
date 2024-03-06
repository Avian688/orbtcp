
import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

cplusplus{{
typedef std::vector<IntMetaData*> IntDataVec;
}}

class IntDataVec { @existingClass; }
//
// This tag specifies In-Network Telemetry (INT) information. It can be attatched to a specific
// region of a packet as a simplified implementation of INT.
//

class IntMetaData extends cObject
{
    string hopName; //may
	simtime_t ts; // Timestamp
	long qLen; // TX Queue Length
	long txBytes; // Transmitted Bytes
	long b; //Link bandwidth capacity
	double averageRtt;
	int numOfFlows;
	long rxQlen; // RX Queue Length
	int numOfFlowsInInitialPhase;
}

class IntTag extends TagBase
{
    long connId;
    IntDataVec intData;
    simtime_t rtt;
    unsigned int cwnd;
	bool initialPhase = true;
}