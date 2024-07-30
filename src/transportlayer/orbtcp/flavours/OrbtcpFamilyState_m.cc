//
// Generated file, do not edit! Created by opp_msgtool 6.0 from transportlayer/orbtcp/flavours/OrbtcpFamilyState.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "OrbtcpFamilyState_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace inet {
namespace tcp {

OrbtcpFamilyStateVariables::OrbtcpFamilyStateVariables()
{
}

void __doPacking(omnetpp::cCommBuffer *b, const OrbtcpFamilyStateVariables& a)
{
    doParsimPacking(b,(::inet::tcp::TcpTahoeRenoFamilyStateVariables&)a);
    doParsimPacking(b,a.B);
    doParsimPacking(b,a.T);
    doParsimPacking(b,a.txRate);
    doParsimPacking(b,a.normalisedInflight);
    doParsimPacking(b,a.normalisedInflightPrev);
    doParsimPacking(b,a.u);
    doParsimPacking(b,a.eta);
    doParsimPacking(b,a.queueingDelay);
    doParsimPacking(b,a.targetUtil);
    doParsimPacking(b,a.incStage);
    doParsimPacking(b,a.maxStage);
    doParsimPacking(b,a.prevWnd);
    doParsimPacking(b,a.additiveIncrease);
    doParsimPacking(b,a.additiveIncreaseGainPerAck);
    doParsimPacking(b,a.lastUpdateSeq);
    doParsimPacking(b,a.R);
    doParsimPacking(b,a.subFlows);
    doParsimPacking(b,a.sharingFlows);
    doParsimPacking(b,a.initialPhaseSharingFlows);
    doParsimPacking(b,a.additiveIncreasePercent);
    doParsimPacking(b,a.rttCount);
    doParsimPacking(b,a.ssthresh);
    doParsimPacking(b,a.initialPhase);
    doParsimPacking(b,a.alpha);
    doParsimPacking(b,a.useHpccAlpha);
    doParsimPacking(b,a.bottBW);
}

void __doUnpacking(omnetpp::cCommBuffer *b, OrbtcpFamilyStateVariables& a)
{
    doParsimUnpacking(b,(::inet::tcp::TcpTahoeRenoFamilyStateVariables&)a);
    doParsimUnpacking(b,a.B);
    doParsimUnpacking(b,a.T);
    doParsimUnpacking(b,a.txRate);
    doParsimUnpacking(b,a.normalisedInflight);
    doParsimUnpacking(b,a.normalisedInflightPrev);
    doParsimUnpacking(b,a.u);
    doParsimUnpacking(b,a.eta);
    doParsimUnpacking(b,a.queueingDelay);
    doParsimUnpacking(b,a.targetUtil);
    doParsimUnpacking(b,a.incStage);
    doParsimUnpacking(b,a.maxStage);
    doParsimUnpacking(b,a.prevWnd);
    doParsimUnpacking(b,a.additiveIncrease);
    doParsimUnpacking(b,a.additiveIncreaseGainPerAck);
    doParsimUnpacking(b,a.lastUpdateSeq);
    doParsimUnpacking(b,a.R);
    doParsimUnpacking(b,a.subFlows);
    doParsimUnpacking(b,a.sharingFlows);
    doParsimUnpacking(b,a.initialPhaseSharingFlows);
    doParsimUnpacking(b,a.additiveIncreasePercent);
    doParsimUnpacking(b,a.rttCount);
    doParsimUnpacking(b,a.ssthresh);
    doParsimUnpacking(b,a.initialPhase);
    doParsimUnpacking(b,a.alpha);
    doParsimUnpacking(b,a.useHpccAlpha);
    doParsimUnpacking(b,a.bottBW);
}

class OrbtcpFamilyStateVariablesDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_B,
        FIELD_T,
        FIELD_txRate,
        FIELD_normalisedInflight,
        FIELD_normalisedInflightPrev,
        FIELD_u,
        FIELD_eta,
        FIELD_queueingDelay,
        FIELD_targetUtil,
        FIELD_incStage,
        FIELD_maxStage,
        FIELD_prevWnd,
        FIELD_additiveIncrease,
        FIELD_additiveIncreaseGainPerAck,
        FIELD_lastUpdateSeq,
        FIELD_R,
        FIELD_subFlows,
        FIELD_sharingFlows,
        FIELD_initialPhaseSharingFlows,
        FIELD_additiveIncreasePercent,
        FIELD_rttCount,
        FIELD_ssthresh,
        FIELD_initialPhase,
        FIELD_alpha,
        FIELD_useHpccAlpha,
        FIELD_bottBW,
    };
  public:
    OrbtcpFamilyStateVariablesDescriptor();
    virtual ~OrbtcpFamilyStateVariablesDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(OrbtcpFamilyStateVariablesDescriptor)

OrbtcpFamilyStateVariablesDescriptor::OrbtcpFamilyStateVariablesDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::tcp::OrbtcpFamilyStateVariables)), "inet::tcp::TcpTahoeRenoFamilyStateVariables")
{
    propertyNames = nullptr;
}

OrbtcpFamilyStateVariablesDescriptor::~OrbtcpFamilyStateVariablesDescriptor()
{
    delete[] propertyNames;
}

bool OrbtcpFamilyStateVariablesDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<OrbtcpFamilyStateVariables *>(obj)!=nullptr;
}

const char **OrbtcpFamilyStateVariablesDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "descriptor",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *OrbtcpFamilyStateVariablesDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "descriptor")) return "readonly";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int OrbtcpFamilyStateVariablesDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 26+base->getFieldCount() : 26;
}

unsigned int OrbtcpFamilyStateVariablesDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_B
        0,    // FIELD_T
        0,    // FIELD_txRate
        0,    // FIELD_normalisedInflight
        0,    // FIELD_normalisedInflightPrev
        0,    // FIELD_u
        0,    // FIELD_eta
        0,    // FIELD_queueingDelay
        0,    // FIELD_targetUtil
        0,    // FIELD_incStage
        0,    // FIELD_maxStage
        0,    // FIELD_prevWnd
        0,    // FIELD_additiveIncrease
        0,    // FIELD_additiveIncreaseGainPerAck
        0,    // FIELD_lastUpdateSeq
        0,    // FIELD_R
        0,    // FIELD_subFlows
        0,    // FIELD_sharingFlows
        0,    // FIELD_initialPhaseSharingFlows
        0,    // FIELD_additiveIncreasePercent
        0,    // FIELD_rttCount
        0,    // FIELD_ssthresh
        0,    // FIELD_initialPhase
        0,    // FIELD_alpha
        0,    // FIELD_useHpccAlpha
        0,    // FIELD_bottBW
    };
    return (field >= 0 && field < 26) ? fieldTypeFlags[field] : 0;
}

const char *OrbtcpFamilyStateVariablesDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "B",
        "T",
        "txRate",
        "normalisedInflight",
        "normalisedInflightPrev",
        "u",
        "eta",
        "queueingDelay",
        "targetUtil",
        "incStage",
        "maxStage",
        "prevWnd",
        "additiveIncrease",
        "additiveIncreaseGainPerAck",
        "lastUpdateSeq",
        "R",
        "subFlows",
        "sharingFlows",
        "initialPhaseSharingFlows",
        "additiveIncreasePercent",
        "rttCount",
        "ssthresh",
        "initialPhase",
        "alpha",
        "useHpccAlpha",
        "bottBW",
    };
    return (field >= 0 && field < 26) ? fieldNames[field] : nullptr;
}

int OrbtcpFamilyStateVariablesDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "B") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "T") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "txRate") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "normalisedInflight") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "normalisedInflightPrev") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "u") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "eta") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "queueingDelay") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "targetUtil") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "incStage") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "maxStage") == 0) return baseIndex + 10;
    if (strcmp(fieldName, "prevWnd") == 0) return baseIndex + 11;
    if (strcmp(fieldName, "additiveIncrease") == 0) return baseIndex + 12;
    if (strcmp(fieldName, "additiveIncreaseGainPerAck") == 0) return baseIndex + 13;
    if (strcmp(fieldName, "lastUpdateSeq") == 0) return baseIndex + 14;
    if (strcmp(fieldName, "R") == 0) return baseIndex + 15;
    if (strcmp(fieldName, "subFlows") == 0) return baseIndex + 16;
    if (strcmp(fieldName, "sharingFlows") == 0) return baseIndex + 17;
    if (strcmp(fieldName, "initialPhaseSharingFlows") == 0) return baseIndex + 18;
    if (strcmp(fieldName, "additiveIncreasePercent") == 0) return baseIndex + 19;
    if (strcmp(fieldName, "rttCount") == 0) return baseIndex + 20;
    if (strcmp(fieldName, "ssthresh") == 0) return baseIndex + 21;
    if (strcmp(fieldName, "initialPhase") == 0) return baseIndex + 22;
    if (strcmp(fieldName, "alpha") == 0) return baseIndex + 23;
    if (strcmp(fieldName, "useHpccAlpha") == 0) return baseIndex + 24;
    if (strcmp(fieldName, "bottBW") == 0) return baseIndex + 25;
    return base ? base->findField(fieldName) : -1;
}

const char *OrbtcpFamilyStateVariablesDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_B
        "omnetpp::simtime_t",    // FIELD_T
        "double",    // FIELD_txRate
        "double",    // FIELD_normalisedInflight
        "double",    // FIELD_normalisedInflightPrev
        "double",    // FIELD_u
        "double",    // FIELD_eta
        "double",    // FIELD_queueingDelay
        "uint32_t",    // FIELD_targetUtil
        "int",    // FIELD_incStage
        "int",    // FIELD_maxStage
        "uint32_t",    // FIELD_prevWnd
        "uint32_t",    // FIELD_additiveIncrease
        "uint32_t",    // FIELD_additiveIncreaseGainPerAck
        "uint32_t",    // FIELD_lastUpdateSeq
        "double",    // FIELD_R
        "int",    // FIELD_subFlows
        "int",    // FIELD_sharingFlows
        "int",    // FIELD_initialPhaseSharingFlows
        "double",    // FIELD_additiveIncreasePercent
        "int",    // FIELD_rttCount
        "uint32_t",    // FIELD_ssthresh
        "bool",    // FIELD_initialPhase
        "double",    // FIELD_alpha
        "double",    // FIELD_useHpccAlpha
        "uint32_t",    // FIELD_bottBW
    };
    return (field >= 0 && field < 26) ? fieldTypeStrings[field] : nullptr;
}

const char **OrbtcpFamilyStateVariablesDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *OrbtcpFamilyStateVariablesDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int OrbtcpFamilyStateVariablesDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void OrbtcpFamilyStateVariablesDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'OrbtcpFamilyStateVariables'", field);
    }
}

const char *OrbtcpFamilyStateVariablesDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string OrbtcpFamilyStateVariablesDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        case FIELD_B: return long2string(pp->B);
        case FIELD_T: return simtime2string(pp->T);
        case FIELD_txRate: return double2string(pp->txRate);
        case FIELD_normalisedInflight: return double2string(pp->normalisedInflight);
        case FIELD_normalisedInflightPrev: return double2string(pp->normalisedInflightPrev);
        case FIELD_u: return double2string(pp->u);
        case FIELD_eta: return double2string(pp->eta);
        case FIELD_queueingDelay: return double2string(pp->queueingDelay);
        case FIELD_targetUtil: return ulong2string(pp->targetUtil);
        case FIELD_incStage: return long2string(pp->incStage);
        case FIELD_maxStage: return long2string(pp->maxStage);
        case FIELD_prevWnd: return ulong2string(pp->prevWnd);
        case FIELD_additiveIncrease: return ulong2string(pp->additiveIncrease);
        case FIELD_additiveIncreaseGainPerAck: return ulong2string(pp->additiveIncreaseGainPerAck);
        case FIELD_lastUpdateSeq: return ulong2string(pp->lastUpdateSeq);
        case FIELD_R: return double2string(pp->R);
        case FIELD_subFlows: return long2string(pp->subFlows);
        case FIELD_sharingFlows: return long2string(pp->sharingFlows);
        case FIELD_initialPhaseSharingFlows: return long2string(pp->initialPhaseSharingFlows);
        case FIELD_additiveIncreasePercent: return double2string(pp->additiveIncreasePercent);
        case FIELD_rttCount: return long2string(pp->rttCount);
        case FIELD_ssthresh: return ulong2string(pp->ssthresh);
        case FIELD_initialPhase: return bool2string(pp->initialPhase);
        case FIELD_alpha: return double2string(pp->alpha);
        case FIELD_useHpccAlpha: return double2string(pp->useHpccAlpha);
        case FIELD_bottBW: return ulong2string(pp->bottBW);
        default: return "";
    }
}

void OrbtcpFamilyStateVariablesDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'OrbtcpFamilyStateVariables'", field);
    }
}

omnetpp::cValue OrbtcpFamilyStateVariablesDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        case FIELD_B: return pp->B;
        case FIELD_T: return pp->T.dbl();
        case FIELD_txRate: return pp->txRate;
        case FIELD_normalisedInflight: return pp->normalisedInflight;
        case FIELD_normalisedInflightPrev: return pp->normalisedInflightPrev;
        case FIELD_u: return pp->u;
        case FIELD_eta: return pp->eta;
        case FIELD_queueingDelay: return pp->queueingDelay;
        case FIELD_targetUtil: return (omnetpp::intval_t)(pp->targetUtil);
        case FIELD_incStage: return pp->incStage;
        case FIELD_maxStage: return pp->maxStage;
        case FIELD_prevWnd: return (omnetpp::intval_t)(pp->prevWnd);
        case FIELD_additiveIncrease: return (omnetpp::intval_t)(pp->additiveIncrease);
        case FIELD_additiveIncreaseGainPerAck: return (omnetpp::intval_t)(pp->additiveIncreaseGainPerAck);
        case FIELD_lastUpdateSeq: return (omnetpp::intval_t)(pp->lastUpdateSeq);
        case FIELD_R: return pp->R;
        case FIELD_subFlows: return pp->subFlows;
        case FIELD_sharingFlows: return pp->sharingFlows;
        case FIELD_initialPhaseSharingFlows: return pp->initialPhaseSharingFlows;
        case FIELD_additiveIncreasePercent: return pp->additiveIncreasePercent;
        case FIELD_rttCount: return pp->rttCount;
        case FIELD_ssthresh: return (omnetpp::intval_t)(pp->ssthresh);
        case FIELD_initialPhase: return pp->initialPhase;
        case FIELD_alpha: return pp->alpha;
        case FIELD_useHpccAlpha: return pp->useHpccAlpha;
        case FIELD_bottBW: return (omnetpp::intval_t)(pp->bottBW);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'OrbtcpFamilyStateVariables' as cValue -- field index out of range?", field);
    }
}

void OrbtcpFamilyStateVariablesDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'OrbtcpFamilyStateVariables'", field);
    }
}

const char *OrbtcpFamilyStateVariablesDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr OrbtcpFamilyStateVariablesDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void OrbtcpFamilyStateVariablesDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    OrbtcpFamilyStateVariables *pp = omnetpp::fromAnyPtr<OrbtcpFamilyStateVariables>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'OrbtcpFamilyStateVariables'", field);
    }
}

}  // namespace tcp
}  // namespace inet

namespace omnetpp {

template<> inet::tcp::OrbtcpFamilyStateVariables *fromAnyPtr(any_ptr ptr) {
    if (ptr.contains<inet::tcp::TcpStateVariables>()) return static_cast<inet::tcp::OrbtcpFamilyStateVariables*>(ptr.get<inet::tcp::TcpStateVariables>());
    if (ptr.contains<omnetpp::cObject>()) return static_cast<inet::tcp::OrbtcpFamilyStateVariables*>(ptr.get<omnetpp::cObject>());
    throw cRuntimeError("Unable to obtain inet::tcp::OrbtcpFamilyStateVariables* pointer from any_ptr(%s)", ptr.pointerTypeName());
}
}  // namespace omnetpp

