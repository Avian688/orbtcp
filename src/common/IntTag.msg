
import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

cplusplus{{
typedef std::vector<IntMetaData> IntDataVec;
}}

class IntDataVec { @existingClass; }
//
// This tag specifies In-Network Telemetry (INT) information. It can be attatched to a specific
// region of a packet as a simplified implementation of INT.
//

class IntMetaData extends cObject
{
    int hopId; //may
	simtime_t ts; // Timestamp
	long qLen; // TX Queue Length
	long txBytes; // Transmitted Bytes
	long b; //Link bandwidth capacity
	double averageRtt;
	int numOfFlows; // Legacy full-INT active-flow estimate
	double effectiveNumOfFlows;
	long rxQlen; // RX Queue Length
	int numOfFlowsInInitialPhase; // Legacy full-INT startup-flow estimate
	int pintPower; // Conceptual encoded PINT field
	double pintUtilization; // Decoded simulation value, not additional modeled wire overhead
	unsigned int pathDigest; // 32-bit order-sensitive OrbCC path fingerprint
	unsigned int queueingDelayCode; // 12-bit cumulative fixed-point queue residence time
	unsigned int pintTotalFlowCountCode; // 8-bit N, present in every PINT record
	unsigned int pintInitialFlowCountCode; // Optional 8-bit S when initialPhase is true
}

class IntTag extends TagBase
{
    long connId;
    IntDataVec intData;
    simtime_t rtt;
    unsigned int cwnd;
	unsigned int pintBaseRttCode; // 24-bit value in microseconds
	unsigned int pintCwndCode; // 16-bit floating-point congestion window in bytes
	bool initialPhase = false; // Set explicitly by the data sender
	bool retrans = false;
}
