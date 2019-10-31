/* Example of a C submodule for Tarantool */
#include <tarantool/module.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "cs104_connection.h"
#include "hal_time.h"
#include "hal_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "ioa_descriptions.h"

#define CONTEXT_DEBUG false

#define OBJECT_ADDRESS "objectAddress"
#define OBJECT_TYPE "objectType"
#define OBJECT_VALUE "value"
#define OBJECT_QUALITY "quality"
#define OBJECT_COT "cot"
#define OBJECT_TIMESTAMP "timestamp"
#define OBJECT_DESCRIPTION "description"
#define QOI "qoi"
#define QCC "qcc"
#define HOST "address" // TODO: change value to "host"
#define DEVICE_ID "device_id"
#define PORT "port"
#define MEASUREMENTS "measurements"

struct context {
    const char *host;
    u_int16_t port;
    bool CONNECTION_CLOSING;
    bool CONNECTION_CLOSED;
    struct json_object *master_object;
};

static void context_dump(struct context *context) {
    fflush(stdout);
    puts("");
    printf("context=%p\n", context);
    printf("host=%s\n", context->host);
    printf("port=%d\n", context->port);
    printf("CONNECTION_CLOSING=%d\n", context->CONNECTION_CLOSING);
    printf("CONNECTION_CLOSED=%d\n", context->CONNECTION_CLOSED);
    printf("master_object=%s\n",
           context->master_object == NULL ? "NULL" : json_object_get_string(context->master_object));
    puts("");
    fflush(stdout);
}

char *ioa_to_string(int ioa) {
    for (unsigned long i = 0; i < sizeof(ioa_descriptions) / sizeof(ioa_descriptions[0]); i++) {
        if (ioa_descriptions[i].ioa == ioa) {
            return ioa_descriptions[i].description;
        }
    }
    return "unknown IOA";
}

static char *QualityToString(QualityDescriptor quality) {
    char buf[80];
    if (quality == IEC60870_QUALITY_GOOD) {
        return strdup("GOOD");
    }
    snprintf(buf, sizeof(buf), "%s%s%s%s%s%s%s",
             (quality & IEC60870_QUALITY_OVERFLOW ? "OVERFLOW|" : ""),
             (quality & IEC60870_QUALITY_RESERVED ? "RESERVED|" : ""),
             (quality & IEC60870_QUALITY_ELAPSED_TIME_INVALID ? "ELAPSED_TIME_INVALID|" : ""),
             (quality & IEC60870_QUALITY_BLOCKED ? "BLOCKED|" : ""),
             (quality & IEC60870_QUALITY_SUBSTITUTED ? "SUBSTITUTED|" : ""),
             (quality & IEC60870_QUALITY_NON_TOPICAL ? "NON_TOPICAL|" : ""),
             (quality & IEC60870_QUALITY_INVALID ? "INVALID|" : "")
    );
    buf[79] = '\0';
    buf[strlen(buf)] = '\0';
    return strdup(buf);
}

/*
M_SP_NA_1	Single-point information
M_SP_TA_1	Single-point information with time tag
M_DP_NA_1	Double-point information
M_DP_TA_1	Double-point information with time tag
M_ST_NA_1	Step position information
M_ST_TA_1	Step position information with time tag
M_BO_NA_1	Bitstring of 32 bit
M_BO_TA_1	Bitstring of 32 bit with time tag
M_ME_NA_1	Measured value, normalised value
M_ME_TA_1	Measured value, normalized value with time tag
M_ME_NB_1	Measured value, scaled value
M_ME_TB_1	Measured value, scaled value wit time tag
M_ME_NC_1	Measured value, short floating point number
M_ME_TC_1	Measured value, short floating point number with time tag
M_IT_NA_1	Integrated totals
M_IT_TA_1	Integrated totals with time tag
M_EP_TA_1	Event of protection equipment with time tag
M_EP_TB_1	Packed start events of protection equipment with time tag
M_EP_TC_1	Packed output circuit information of protection equipment with time tag
M_PS_NA_1	Packed single point information with status change detection
M_ME_ND_1	Measured value, normalized value without quality descriptor
M_SP_TB_1	Single-point information with time tag CP56Time2a
M_DP_TB_1	Double-point information with time tag CP56Time2a
M_ST_TB_1	Step position information with time tag CP56Time2a
M_BO_TB_1	Bitstring of 32 bit with time tag CP56Time2a
M_ME_TD_1	Measured value, normalised value with time tag CP56Time2a
M_ME_TE_1	Measured value, scaled value with time tag CP56Time2a
M_ME_TF_1	Measured value, short floating point number with time tag CP56Time2a
M_IT_TB_1	Integrated totals with time tag CP56Time2a
M_EP_TD_1	Event of protection equipment with time tag CP56Time2a
M_EP_TE_1	Packed start events of protection equipment with time tag CP56Time2a
M_EP_TF_1	Packed output circuit information of protection equipment with time tag CP56Time2a
C_SC_NA_1	Single command
C_DC_NA_1	Double command
C_RC_NA_1	Regulating step command
C_SE_NA_1	Set-point Command, normalised value
C_SE_NB_1	Set-point Command, scaled value
C_SE_NC_1	Set-point Command, short floating point number
C_BO_NA_1	Bitstring 32 bit command
C_SC_TA_1	Single command with time tag CP56Time2a
C_DC_TA_1	Double command with time tag CP56Time2a
C_RC_TA_1	Regulating step command with time tag CP56Time2a
C_SE_TA_1	Measured value, normalised value command with time tag CP56Time2a
C_SE_TB_1	Measured value, scaled value command with time tag CP56Time2a
C_SE_TC_1	Measured value, short floating point number command with time tag CP56Time2a
C_BO_TA_1	Bitstring of 32 bit command with time tag CP56Time2a
M_EI_NA_1	End of Initialisation
C_IC_NA_1	Interrogation command
C_CI_NA_1	Counter interrogation command
C_RD_NA_1	Read command
C_CS_NA_1	Clock synchronisation command
C_TS_NA_1	Test command
C_RP_NA_1	Reset process command
C_CD_NA_1	Delay acquisition command
C_TS_TA_1	Test command with time tag CP56Time2a
P_ME_NA_1	Parameter of measured values, normalized value
P_ME_NB_1	Parameter of measured values, scaled value
P_ME_NC_1	Parameter of measured values, short floating point number
P_AC_NA_1	Parameter activation
F_FR_NA_1	File ready
F_SR_NA_1	Section ready
F_SC_NA_1	Call directory, select file, call file, call section
F_LS_NA_1	Last section, last segment
F_FA_NA_1	ACK file, ACK section
F_SG_NA_1	Segment
F_DR_TA_1	Directory
 */

static int64_t currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    int64_t s1 = (int64_t) (time.tv_sec) * 1000;
    int64_t s2 = (time.tv_usec / 1000);
    return s1 + s2;
}

static struct json_object *master_object_create(struct context *context) {
    json_object *master_object = json_object_new_object();
    if (CONTEXT_DEBUG) {
        context_dump(context);
    }
    json_object_object_add(master_object, HOST, json_object_new_string(context->host));
    json_object_object_add(master_object, PORT, json_object_new_int(context->port));
    json_object_object_add(master_object, OBJECT_TIMESTAMP, json_object_new_int64(currentTimeMillis()));
    json_object_object_add(master_object, MEASUREMENTS, json_object_new_array());
    return master_object;
}

static void put_measurement(struct context *context, struct json_object *measurement) {
    struct json_object *master_object = context->master_object;
    struct json_object *ioa;
    json_object_object_get_ex(measurement, OBJECT_ADDRESS, &ioa);
    int ioa_int = json_object_get_int(ioa);

    struct json_object *desc = json_object_new_string(ioa_to_string(ioa_int));
    json_object_object_add(measurement, OBJECT_DESCRIPTION, desc);

    struct json_object *measurements_array;
    json_object_object_get_ex(master_object, MEASUREMENTS, &measurements_array);

    int replaced = 0;
    for (int i = 0; i < json_object_array_length(measurements_array); i++) {
        struct json_object *cur_measurement = json_object_array_get_idx(measurements_array, i);
        struct json_object *cur_ioa;
        json_object_object_get_ex(cur_measurement, OBJECT_ADDRESS, &cur_ioa);
        int cur_ioa_int = json_object_get_int(cur_ioa);
        if (cur_ioa_int == ioa_int) {
            json_object_array_put_idx(measurements_array, i, measurement);
            replaced = 1;
        }
    }
    if (!replaced) {
        json_object_array_add(measurements_array, measurement);
    }
}

// M_SP_NA_1: "Single-point information"
static void jsonify_M_SP_NA_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE,
                               json_object_new_boolean(SinglePointInformation_getValue((SinglePointInformation) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        put_measurement(context, measurement);
        SinglePointInformation_destroy((SinglePointInformation) io);
    }
}

// M_BO_NA_1: "Bitstring of 32 bit"
static void jsonify_M_BO_NA_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        char value[11];
        snprintf(value, sizeof(value), "0x%08X", BitString32_getValue((BitString32) io));

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));

        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_string(value));

        if (InformationObject_getObjectAddress(io) == 1000 ){
            json_object_object_add(context->master_object, DEVICE_ID, json_object_new_string(value));
        }

        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(context, measurement);
        BitString32_destroy((BitString32) io);
    }
}

// M_ME_NB_1: "Measured value, scaled value"
static void jsonify_M_ME_NB_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE,
                               json_object_new_int(MeasuredValueScaled_getValue((MeasuredValueScaled) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(MeasuredValueScaled_getQuality((MeasuredValueScaled) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(context, measurement);
        MeasuredValueScaled_destroy((MeasuredValueScaled) io);
    }
}

// M_ME_NC_1: "Measured value, short floating point number"
static void jsonify_M_ME_NC_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);
        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE,
                               json_object_new_int(MeasuredValueShort_getValue((MeasuredValueShort) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(context, measurement);
        MeasuredValueShort_destroy((MeasuredValueShort) io);
    }
}

// M_SP_TB_1: "Single-point information with time tag CP56Time2a"
static void jsonify_M_SP_TB_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_TIMESTAMP, json_object_new_int64(
                CP56Time2a_toMsTimestamp(SinglePointWithCP56Time2a_getTimestamp((SinglePointWithCP56Time2a) io))));
        json_object_object_add(measurement, OBJECT_VALUE,
                               json_object_new_boolean(SinglePointInformation_getValue((SinglePointInformation) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(SinglePointInformation_getQuality((SinglePointInformation) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(context, measurement);
        SinglePointWithCP56Time2a_destroy((SinglePointWithCP56Time2a) io);
    }
}

// M_ME_TF_1: "Measured value, short floating point number with time tag CP56Time2a"
static void jsonify_M_ME_TF_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_TIMESTAMP, json_object_new_int64(CP56Time2a_toMsTimestamp(
                MeasuredValueShortWithCP56Time2a_getTimestamp((MeasuredValueShortWithCP56Time2a) io))));
        json_object_object_add(measurement, OBJECT_VALUE,
                               json_object_new_int(MeasuredValueShort_getValue((MeasuredValueShort) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(context, measurement);
        MeasuredValueShortWithCP56Time2a_destroy((MeasuredValueShortWithCP56Time2a) io);
    }
}

// C_IC_NA_1: "Interrogation command"
static void jsonify_C_IC_NA_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, QOI,
                               json_object_new_int(InterrogationCommand_getQOI((InterrogationCommand) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        put_measurement(context, measurement);
        InterrogationCommand_destroy((InterrogationCommand) io);
    }
}

// C_CI_NA_1: "Counter interrogation command"
static void jsonify_C_CI_NA_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, QCC, json_object_new_int(
                CounterInterrogationCommand_getQCC((CounterInterrogationCommand) io)));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        put_measurement(context, measurement);
        CounterInterrogationCommand_destroy((CounterInterrogationCommand) io);
    }
}

// C_CS_NA_1: "Clock synchronisation command"
static void jsonify_C_CS_NA_1(struct sCS101_ASDU *asdu, struct context *context) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE,
                               json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS,
                               json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_TIMESTAMP, json_object_new_int64(
                CP56Time2a_toMsTimestamp(ClockSynchronizationCommand_getTime((ClockSynchronizationCommand) io))));
        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        put_measurement(context, measurement);
        ClockSynchronizationCommand_destroy((ClockSynchronizationCommand) io);
    }
}

/* Connection event handler */
static void
connectionHandler(void *parameter, CS104_Connection connection, CS104_ConnectionEvent event) {
    struct context* context = parameter;
    if(CONTEXT_DEBUG) {
        context_dump(context);
    }
    switch (event) {
        case CS104_CONNECTION_OPENED:
            printf("%s:%d Connection established\n", context->host, context->port);
            CS104_Connection_sendStartDT(connection);
            break;
        case CS104_CONNECTION_CLOSED:
            printf("%s:%d Connection closed\n", context->host, context->port);
            context->CONNECTION_CLOSED = true;
            break;
        case CS104_CONNECTION_STARTDT_CON_RECEIVED:
            printf("%s:%d Received STARTDT_CON\n", context->host, context->port);
            CS104_Connection_sendInterrogationCommand(connection, CS101_COT_ACTIVATION, 1, IEC60870_QOI_STATION);
//            CS104_Connection_sendCounterInterrogationCommand(connection, IEC60870_QCC_RQT_GENERAL, 1, IEC60870_QOI_STATION); // TODO: Verify
            break;
        case CS104_CONNECTION_STOPDT_CON_RECEIVED:
            printf("%s:%d Received STOPDT_CON - closing connection\n", context->host, context->port);
            context->CONNECTION_CLOSING = true;
            CS104_Connection_close(connection);
            break;
        default:
            fprintf(stderr, "%s:%d ERROR: Received unknown event %d\n", context->host, context->port, event);
            exit(EXIT_FAILURE);
    }
}

/*
 * CS101_ASDUReceivedHandler implementation
 *
 * For CS104 the address parameter has to be ignored
 */
static bool
asduReceivedHandler(void *parameter, int address, CS101_ASDU asdu) {
    (void) address;
    const int type = CS101_ASDU_getTypeID(asdu);
    const char *typeStr = TypeID_toString(type);
    const int cot = CS101_ASDU_getCOT(asdu);
    printf("RECVD ASDU type: %s(%i) elements: %i cot %s(%i)\n",
           typeStr,
           type,
           CS101_ASDU_getNumberOfElements(asdu),
           CS101_CauseOfTransmission_toString(cot),
           cot);

    struct context *context = parameter;
    if (cot == CS101_COT_ACTIVATION_TERMINATION) {
        context->CONNECTION_CLOSING = true;
        //exit(EXIT_SUCCESS);
    }

    switch (type) {
        case M_SP_NA_1: // "Single-point information"
            jsonify_M_SP_NA_1(asdu, context);
            break;
//        case M_SP_TA_1: // "Single-point information with time tag"
//            jsonify_M_SP_TA_1(asdu, context);
//            break;
//        case M_DP_NA_1: // "Double-point information"
//            jsonify_M_DP_NA_1(asdu, context);
//            break;
//        case M_DP_TA_1: // "Double-point information with time tag"
//            jsonify_M_DP_TA_1(asdu, context);
//            break;
//        case M_ST_NA_1: // "Step position information"
//            jsonify_M_ST_NA_1(asdu, context);
//            break;
//        case M_ST_TA_1: // "Step position information with time tag"
//            jsonify_M_ST_TA_1(asdu, context);
//            break;
        case M_BO_NA_1: // "Bitstring of 32 bit"
            jsonify_M_BO_NA_1(asdu, context);
            break;
//        case M_BO_TA_1: // "Bitstring of 32 bit with time tag"
//            jsonify_M_BO_TA_1(asdu, context);
//            break;
//        case M_ME_NA_1: // "Measured value, normalised value"
//            jsonify_M_ME_NA_1(asdu, context);
//            break;
//        case M_ME_TA_1: // "Measured value, normalized value with time tag"
//            jsonify_M_ME_TA_1(asdu, context);
//            break;
        case M_ME_NB_1: // "Measured value, scaled value"
            jsonify_M_ME_NB_1(asdu, context);
            break;
//        case M_ME_TB_1: // "Measured value, scaled value wit time tag"
//            jsonify_M_ME_TB_1(asdu, context);
//            break;
        case M_ME_NC_1: // "Measured value, short floating point number"
            jsonify_M_ME_NC_1(asdu, context);
            break;
//        case M_ME_TC_1: // "Measured value, short floating point number with time tag"
//            jsonify_M_ME_TC_1(asdu, context);
//            break;
//        case M_IT_NA_1: // "Integrated totals"
//            jsonify_M_IT_NA_1(asdu, context);
//            break;
//        case M_IT_TA_1: // "Integrated totals with time tag"
//            jsonify_M_IT_TA_1(asdu, context);
//            break;
//        case M_EP_TA_1: // "Event of protection equipment with time tag"
//            jsonify_M_EP_TA_1(asdu, context);
//            break;
//        case M_EP_TB_1: // "Packed start events of protection equipment with time tag"
//            jsonify_M_EP_TB_1(asdu, context);
//            break;
//        case M_EP_TC_1: // "Packed output circuit information of protection equipment with time tag"
//            jsonify_M_EP_TC_1(asdu, context);
//            break;
//        case M_PS_NA_1: // "Packed single point information with status change detection"
//            jsonify_M_PS_NA_1(asdu, context);
//            break;
//        case M_ME_ND_1: // "Measured value, normalized value without quality descriptor"
//            jsonify_M_ME_ND_1(asdu, context);
//            break;
        case M_SP_TB_1: // "Single-point information with time tag CP56Time2a"
            jsonify_M_SP_TB_1(asdu, context);
            break;
//        case M_DP_TB_1: // "Double-point information with time tag CP56Time2a"
//            jsonify_M_DP_TB_1(asdu, context);
//            break;
//        case M_ST_TB_1: // "Step position information with time tag CP56Time2a"
//            jsonify_M_ST_TB_1(asdu, context);
//            break;
//        case M_BO_TB_1: // "Bitstring of 32 bit with time tag CP56Time2a"
//            jsonify_M_BO_TB_1(asdu, context);
//            break;
//        case M_ME_TD_1: // "Measured value, normalised value with time tag CP56Time2a"
//            jsonify_M_ME_TD_1(asdu, context);
//            break;
//        case M_ME_TE_1: // "Measured value, scaled value with time tag CP56Time2a"
//            jsonify_M_ME_TE_1(asdu, context);
//            break;
        case M_ME_TF_1: // "Measured value, short floating point number with time tag CP56Time2a"
            jsonify_M_ME_TF_1(asdu, context);
            break;
//        case M_IT_TB_1: // "Integrated totals with time tag CP56Time2a"
//            jsonify_M_IT_TB_1(asdu, context);
//            break;
//        case M_EP_TD_1: // "Event of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TD_1(asdu, context);
//            break;
//        case M_EP_TE_1: // "Packed start events of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TE_1(asdu, context);
//            break;
//        case M_EP_TF_1: // "Packed output circuit information of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TF_1(asdu, context);
//            break;
//        case C_SC_NA_1: // "Single command"
//            jsonify_C_SC_NA_1(asdu, context);
//            break;
//        case C_DC_NA_1: // "Double command"
//            jsonify_C_DC_NA_1(asdu, context);
//            break;
//        case C_RC_NA_1: // "Regulating step command"
//            jsonify_C_RC_NA_1(asdu, context);
//            break;
//        case C_SE_NA_1: // "Set-point Command, normalised value"
//            jsonify_C_SE_NA_1(asdu, context);
//            break;
//        case C_SE_NB_1: // "Set-point Command, scaled value"
//            jsonify_C_SE_NB_1(asdu, context);
//            break;
//        case C_SE_NC_1: // "Set-point Command, short floating point number"
//            jsonify_C_SE_NC_1(asdu, context);
//            break;
//        case C_BO_NA_1: // "Bitstring 32 bit command"
//            jsonify_C_BO_NA_1(asdu, context);
//            break;
//        case C_SC_TA_1: // "Single command with time tag CP56Time2a"
//            jsonify_C_SC_TA_1(asdu, context);
//            break;
//        case C_DC_TA_1: // "Double command with time tag CP56Time2a"
//            jsonify_C_DC_TA_1(asdu, context);
//            break;
//        case C_RC_TA_1: // "Regulating step command with time tag CP56Time2a"
//            jsonify_C_RC_TA_1(asdu, context);
//            break;
//        case C_SE_TA_1: // "Measured value, normalised value command with time tag CP56Time2a"
//            jsonify_C_SE_TA_1(asdu, context);
//            break;
//        case C_SE_TB_1: // "Measured value, scaled value command with time tag CP56Time2a"
//            jsonify_C_SE_TB_1(asdu, context);
//            break;
//        case C_SE_TC_1: // "Measured value, short floating point number command with time tag CP56Time2a"
//            jsonify_C_SE_TC_1(asdu, context);
//            break;
//        case C_BO_TA_1: // "Bitstring of 32 bit command with time tag CP56Time2a"
//            jsonify_C_BO_TA_1(asdu, context);
//            break;
//        case M_EI_NA_1: // "End of Initialisation"
//            jsonify_M_EI_NA_1(asdu, context);
//            break;
        case C_IC_NA_1: // "Interrogation command"
            jsonify_C_IC_NA_1(asdu, context);
            break;
        case C_CI_NA_1: // "Counter interrogation command"
            jsonify_C_CI_NA_1(asdu, context);
            break;
//        case C_RD_NA_1: // "Read command"
//            jsonify_C_RD_NA_1(asdu, context);
//            break;
        case C_CS_NA_1: // "Clock synchronisation command"
            jsonify_C_CS_NA_1(asdu, context);
            break;
//        case C_TS_NA_1: // "Test command"
//            jsonify_C_TS_NA_1(asdu, context);
//            break;
//        case C_RP_NA_1: // "Reset process command"
//            jsonify_C_RP_NA_1(asdu, context);
//            break;
//        case C_CD_NA_1: // "Delay acquisition command"
//            jsonify_C_CD_NA_1(asdu, context);
//            break;
//        case C_TS_TA_1: // "Test command with time tag CP56Time2a"
//            jsonify_C_TS_TA_1(asdu, context);
//            break;
//        case P_ME_NA_1: // "Parameter of measured values, normalized value"
//            jsonify_P_ME_NA_1(asdu, context);
//            break;
//        case P_ME_NB_1: // "Parameter of measured values, scaled value"
//            jsonify_P_ME_NB_1(asdu, context);
//            break;
//        case P_ME_NC_1: // "Parameter of measured values, short floating point number"
//            jsonify_P_ME_NC_1(asdu, context);
//            break;
//        case P_AC_NA_1: // "Parameter activation"
//            jsonify_P_AC_NA_1(asdu, context);
//            break;
//        case F_FR_NA_1: // "File ready"
//            jsonify_F_FR_NA_1(asdu, context);
//            break;
//        case F_SR_NA_1: // "Section ready"
//            jsonify_F_SR_NA_1(asdu, context);
//            break;
//        case F_SC_NA_1: // "Call directory, select file, call file, call section"
//            jsonify_F_SC_NA_1(asdu, context);
//            break;
//        case F_LS_NA_1: // "Last section, last segment"
//            jsonify_F_LS_NA_1(asdu, context);
//            break;
//        case F_FA_NA_1: // "ACK file, ACK section"
//            jsonify_F_FA_NA_1(asdu, context);
//            break;
//        case F_SG_NA_1: // "Segment"
//            jsonify_F_SG_NA_1(asdu, context);
//            break;
//        case F_DR_TA_1: // "Directory"
//            jsonify_F_DR_TA_1(asdu, context);
//            break;
        default:
            fprintf(stderr, "Got not implemented yet ASDU type %s(%d)\n", typeStr, type);
            //exit(1);
    }
    return true;
}

static int
iec_104_fetch(struct lua_State *L) {
#if defined(STANDALONE)
	(void) L;
	return 0;
}

int main(int argc, char **argv) {
#endif
    struct context context = {};

#if defined(STANDALONE)
    if (argc < 3) {
		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	context.host = argv[1];
	context.port = (uint16_t)strtol(argv[2], NULL, 10);
#else
    if (lua_gettop(L) < 2)
    		luaL_error(L, "Usage: fetch(host: string, port: number)");

    context.host = lua_tostring(L, 1);
    context.port = lua_tointeger(L, 2);
#endif

    if (CONTEXT_DEBUG) {
        context_dump(&context);
    }

    //printf("Connecting to: %s:%i\n", context.host, context.port);
    CS104_Connection con = CS104_Connection_create(context.host, context.port);
    context.master_object = master_object_create(&context);
    CS104_Connection_setConnectionHandler(con, connectionHandler, &context);
    CS104_Connection_setASDUReceivedHandler(con, asduReceivedHandler, &context);

    /* uncomment to log messages */
    //CS104_Connection_setRawMessageHandler(con, rawMessageHandler, NULL);
    if(CS104_Connection_connect(con)) {
        long int time_start;
        long int time_current;
        time_start = time(NULL);
        while (!context.CONNECTION_CLOSING) {
            Thread_sleep(100);
            time_current = time(NULL);
            if (time_current - time_start > 15) {
                printf("Timed out receiving data\n");
                break;
            }
        }
        printf("Sending StopDT\n");
        CS104_Connection_sendStopDT(con);
        while (!context.CONNECTION_CLOSED) {
            Thread_sleep(100);
            time_current = time(NULL);
            if (time_current - time_start > 15) {
                printf("Timed out closing the connection\n");
                break;
            }
        }
        printf("Destroying the connection\n");
        CS104_Connection_destroy(con);
        printf("Destroyed the connection\n");
    }
    else {

    }

    const char *json_string = json_object_get_string(context.master_object);

#if defined(STANDALONE)
    printf("%s\n", json_string);
#else
    lua_pushstring(L, json_string);
#endif

    json_object_put(context.master_object);
    context.master_object = NULL;


    return 1;
    //printf("exit\n");
}


/* exported function */
LUA_API int
luaopen_ckit_lib(lua_State *L)
{
	/* result returned from require('ckit.lib') */
	lua_newtable(L);
	static const struct luaL_Reg meta [] = {
		{"fetch", iec_104_fetch},
		{NULL, NULL}
	};
	luaL_register(L, NULL, meta);
	return 1;
}
/* vim: syntax=c ts=8 sts=8 sw=8 noet */
