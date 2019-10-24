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

#define OBJECT_ADDRESS "objectAddress"
#define OBJECT_TYPE "objectType"
#define OBJECT_VALUE "value"
#define OBJECT_QUALITY "quality"
#define OBJECT_COT "cot"
#define OBJECT_TIMESTAMP "timestamp"
#define OBJECT_DESCRIPTION "description"
#define QOI "qoi"
#define QCC "qcc"
#define ADDRESS "address"
#define DEVICE_ID "device_id"
#define PORT "port"
#define MEASUREMENTS "measurements"

struct channel_desc_entry {
    int channelId;
    char *channelDesc;
};

bool CONNECTION_CLOSING_FLAG = false;

struct channel_desc_entry entries[] = {
        {100,  "Power Active channel 1"},
        {116,  "Power Active channel 2"},
        {132,  "Power Active channel 3"},
        {148,  "Power Active channel 4"},
        {164,  "Power Active channel 5"},
        {180,  "Power Active channel 6"},
        {196,  "Power Active channel 7"},
        {212,  "Power Active channel 8"},
        {101,  "Power Reactive channel 1"},
        {117,  "Power Reactive channel 2"},
        {133,  "Power Reactive channel 3"},
        {149,  "Power Reactive channel 4"},
        {165,  "Power Reactive channel 5"},
        {181,  "Power Reactive channel 6"},
        {197,  "Power Reactive channel 7"},
        {213,  "Power Reactive channel 8"},
        {102,  "Power Apparent channel 1"},
        {118,  "Power Apparent channel 2"},
        {134,  "Power Apparent channel 3"},
        {150,  "Power Apparent channel 4"},
        {166,  "Power Apparent channel 5"},
        {182,  "Power Apparent channel 6"},
        {198,  "Power Apparent channel 7"},
        {214,  "Power Apparent channel 8"},
        {103,  "RMS voltage channel 1"},
        {119,  "RMS voltage channel 2"},
        {135,  "RMS voltage channel 3"},
        {151,  "RMS voltage channel 4"},
        {167,  "RMS voltage channel 5"},
        {183,  "RMS voltage channel 6"},
        {199,  "RMS voltage channel 7"},
        {215,  "RMS voltage channel 8"},
        {104,  "RMS current channel 1"},
        {120,  "RMS current channel 2"},
        {136,  "RMS current channel 3"},
        {152,  "RMS current channel 4"},
        {168,  "RMS current channel 5"},
        {184,  "RMS current channel 6"},
        {200,  "RMS current channel 7"},
        {216,  "RMS current channel 8"},
        {105,  "Frequency, Hz channel 1"},
        {121,  "Frequency, Hz channel 2"},
        {137,  "Frequency, Hz channel 3"},
        {153,  "Frequency, Hz channel 4"},
        {169,  "Frequency, Hz channel 5"},
        {185,  "Frequency, Hz channel 6"},
        {201,  "Frequency, Hz channel 7"},
        {217,  "Frequency, Hz channel 8"},
        {106,  "phase-delay, degree channel 1"},
        {122,  "phase-delay, degree channel 2"},
        {138,  "phase-delay, degree channel 3"},
        {154,  "phase-delay, degree channel 4"},
        {170,  "phase-delay, degree channel 5"},
        {186,  "phase-delay, degree channel 6"},
        {202,  "phase-delay, degree channel 7"},
        {218,  "phase-delay, degree channel 8"},
        {107,  "Voltage SAG Time channel 1"},
        {123,  "Voltage SAG Time channel 2"},
        {139,  "Voltage SAG Time channel 3"},
        {155,  "Voltage SAG Time channel 4"},
        {171,  "Voltage SAG Time channel 5"},
        {187,  "Voltage SAG Time channel 6"},
        {203,  "Voltage SAG Time channel 7"},
        {219,  "Voltage SAG Time channel 8"},
        {108,  "Voltage Swell Time channel 1"},
        {124,  "Voltage Swell Time channel 2"},
        {140,  "Voltage Swell Time channel 3"},
        {156,  "Voltage Swell Time channel 4"},
        {172,  "Voltage Swell Time channel 5"},
        {188,  "Voltage Swell Time channel 6"},
        {204,  "Voltage Swell Time channel 7"},
        {220,  "Voltage Swell Time channel 8"},
        {109,  "Current Swell Time channel 1"},
        {125,  "Current Swell Time channel 2"},
        {141,  "Current Swell Time channel 3"},
        {157,  "Current Swell Time channel 4"},
        {173,  "Current Swell Time channel 5"},
        {189,  "Current Swell Time channel 6"},
        {205,  "Current Swell Time channel 7"},
        {221,  "Current Swell Time channel 8"},
        {400,  "DetectVoltage channel 1"},
        {416,  "DetectVoltage channel 2"},
        {432,  "DetectVoltage channel 3"},
        {448,  "DetectVoltage channel 4"},
        {464,  "DetectVoltage channel 5"},
        {480,  "DetectVoltage channel 6"},
        {496,  "DetectVoltage channel 7"},
        {512,  "DetectVoltage channel 8"},
        {401,  "UnderVoltage channel 1"},
        {417,  "UnderVoltage channel 2"},
        {433,  "UnderVoltage channel 3"},
        {449,  "UnderVoltage channel 4"},
        {465,  "UnderVoltage channel 5"},
        {481,  "UnderVoltage channel 6"},
        {497,  "UnderVoltage channel 7"},
        {513,  "UnderVoltage channel 8"},
        {402,  "OverVoltage channel 1"},
        {418,  "OverVoltage channel 2"},
        {434,  "OverVoltage channel 3"},
        {450,  "OverVoltage channel 4"},
        {466,  "OverVoltage channel 5"},
        {482,  "OverVoltage channel 6"},
        {498,  "OverVoltage channel 7"},
        {514,  "OverVoltage channel 8"},
        {1000, "ID device0"},
        {1001, "ID device1"},
        {1002, "ID device2"},
        {1003, "Manufacture & Pcb revision 0x0000&0x0000"},
        {1004, "Firmware Major.Minor 0x0000.0x0000"},
        {1010, "Напряжение питания МК, мВ"},
        {1011, "Температура МК, град С"},
        {1012, "Напряжение питания модема, мВ"},
        {4000, "T0 Energy Active forward channel 1"},
        {4004, "T0 Energy Active forward channel 2"},
        {4008, "T0 Energy Active forward channel 3"},
        {4012, "T0 Energy Active forward channel 4"},
        {4016, "T0 Energy Active forward channel 5"},
        {4020, "T0 Energy Active forward channel 6"},
        {4024, "T0 Energy Active forward channel 7"},
        {4028, "T0 Energy Active forward channel 8"},
        {4001, "T0 Energy Reactive forward channel 1"},
        {4005, "T0 Energy Reactive forward channel 2"},
        {4009, "T0 Energy Reactive forward channel 3"},
        {4013, "T0 Energy Reactive forward channel 4"},
        {4017, "T0 Energy Reactive forward channel 5"},
        {4021, "T0 Energy Reactive forward channel 6"},
        {4025, "T0 Energy Reactive forward channel 7"},
        {4029, "T0 Energy Reactive forward channel 8"},
        {4002, "T0 Energy Active reverse channel 1"},
        {4006, "T0 Energy Active reverse channel 2"},
        {4010, "T0 Energy Active reverse channel 3"},
        {4014, "T0 Energy Active reverse channel 4"},
        {4018, "T0 Energy Active reverse channel 5"},
        {4022, "T0 Energy Active reverse channel 6"},
        {4026, "T0 Energy Active reverse channel 7"},
        {4030, "T0 Energy Active reverse channel 8"},
        {4003, "T0 Energy Reactive reverse channel 1"},
        {4007, "T0 Energy Reactive reverse channel 2"},
        {4011, "T0 Energy Reactive reverse channel 3"},
        {4015, "T0 Energy Reactive reverse channel 4"},
        {4019, "T0 Energy Reactive reverse channel 5"},
        {4023, "T0 Energy Reactive reverse channel 6"},
        {4027, "T0 Energy Reactive reverse channel 7"},
        {4031, "T0 Energy Reactive reverse channel 8"},
        {4100, "T1 Energy Active forward channel 1"},
        {4104, "T1 Energy Active forward channel 2"},
        {4108, "T1 Energy Active forward channel 3"},
        {4112, "T1 Energy Active forward channel 4"},
        {4116, "T1 Energy Active forward channel 5"},
        {4120, "T1 Energy Active forward channel 6"},
        {4124, "T1 Energy Active forward channel 7"},
        {4128, "T1 Energy Active forward channel 8"},
        {4101, "T1 Energy Reactive forward channel 1"},
        {4105, "T1 Energy Reactive forward channel 2"},
        {4109, "T1 Energy Reactive forward channel 3"},
        {4113, "T1 Energy Reactive forward channel 4"},
        {4117, "T1 Energy Reactive forward channel 5"},
        {4121, "T1 Energy Reactive forward channel 6"},
        {4125, "T1 Energy Reactive forward channel 7"},
        {4129, "T1 Energy Reactive forward channel 8"},
        {4102, "T1 Energy Active reverse channel 1"},
        {4106, "T1 Energy Active reverse channel 2"},
        {4110, "T1 Energy Active reverse channel 3"},
        {4114, "T1 Energy Active reverse channel 4"},
        {4118, "T1 Energy Active reverse channel 5"},
        {4122, "T1 Energy Active reverse channel 6"},
        {4126, "T1 Energy Active reverse channel 7"},
        {4130, "T1 Energy Active reverse channel 8"},
        {4103, "T1 Energy Reactive reverse channel 1"},
        {4107, "T1 Energy Reactive reverse channel 2"},
        {4111, "T1 Energy Reactive reverse channel 3"},
        {4115, "T1 Energy Reactive reverse channel 4"},
        {4119, "T1 Energy Reactive reverse channel 5"},
        {4123, "T1 Energy Reactive reverse channel 6"},
        {4127, "T1 Energy Reactive reverse channel 7"},
        {4131, "T1 Energy Reactive reverse channel 8"},
        {4200, "T2 Energy Active forward channel 1"},
        {4204, "T2 Energy Active forward channel 2"},
        {4208, "T2 Energy Active forward channel 3"},
        {4212, "T2 Energy Active forward channel 4"},
        {4216, "T2 Energy Active forward channel 5"},
        {4220, "T2 Energy Active forward channel 6"},
        {4224, "T2 Energy Active forward channel 7"},
        {4228, "T2 Energy Active forward channel 8"},
        {4201, "T2 Energy Reactive forward channel 1"},
        {4205, "T2 Energy Reactive forward channel 2"},
        {4209, "T2 Energy Reactive forward channel 3"},
        {4213, "T2 Energy Reactive forward channel 4"},
        {4217, "T2 Energy Reactive forward channel 5"},
        {4221, "T2 Energy Reactive forward channel 6"},
        {4225, "T2 Energy Reactive forward channel 7"},
        {4229, "T2 Energy Reactive forward channel 8"},
        {4202, "T2 Energy Active reverse channel 1"},
        {4206, "T2 Energy Active reverse channel 2"},
        {4210, "T2 Energy Active reverse channel 3"},
        {4214, "T2 Energy Active reverse channel 4"},
        {4218, "T2 Energy Active reverse channel 5"},
        {4222, "T2 Energy Active reverse channel 6"},
        {4226, "T2 Energy Active reverse channel 7"},
        {4230, "T2 Energy Active reverse channel 8"},
        {4203, "T2 Energy Reactive reverse channel 1"},
        {4207, "T2 Energy Reactive reverse channel 2"},
        {4211, "T2 Energy Reactive reverse channel 3"},
        {4215, "T2 Energy Reactive reverse channel 4"},
        {4219, "T2 Energy Reactive reverse channel 5"},
        {4223, "T2 Energy Reactive reverse channel 6"},
        {4227, "T2 Energy Reactive reverse channel 7"},
        {4231, "T2 Energy Reactive reverse channel 8"},
        {4300, "T3 Energy Active forward channel 1"},
        {4304, "T3 Energy Active forward channel 2"},
        {4308, "T3 Energy Active forward channel 3"},
        {4312, "T3 Energy Active forward channel 4"},
        {4316, "T3 Energy Active forward channel 5"},
        {4320, "T3 Energy Active forward channel 6"},
        {4324, "T3 Energy Active forward channel 7"},
        {4328, "T3 Energy Active forward channel 8"},
        {4301, "T3 Energy Reactive forward channel 1"},
        {4305, "T3 Energy Reactive forward channel 2"},
        {4309, "T3 Energy Reactive forward channel 3"},
        {4313, "T3 Energy Reactive forward channel 4"},
        {4317, "T3 Energy Reactive forward channel 5"},
        {4321, "T3 Energy Reactive forward channel 6"},
        {4325, "T3 Energy Reactive forward channel 7"},
        {4329, "T3 Energy Reactive forward channel 8"},
        {4302, "T3 Energy Active reverse channel 1"},
        {4306, "T3 Energy Active reverse channel 2"},
        {4310, "T3 Energy Active reverse channel 3"},
        {4314, "T3 Energy Active reverse channel 4"},
        {4318, "T3 Energy Active reverse channel 5"},
        {4322, "T3 Energy Active reverse channel 6"},
        {4326, "T3 Energy Active reverse channel 7"},
        {4330, "T3 Energy Active reverse channel 8"},
        {4303, "T3 Energy Reactive reverse channel 1"},
        {4307, "T3 Energy Reactive reverse channel 2"},
        {4311, "T3 Energy Reactive reverse channel 3"},
        {4315, "T3 Energy Reactive reverse channel 4"},
        {4319, "T3 Energy Reactive reverse channel 5"},
        {4323, "T3 Energy Reactive reverse channel 6"},
        {4327, "T3 Energy Reactive reverse channel 7"},
        {4331, "T3 Energy Reactive reverse channel 8"},
        {4400, "T4 Energy Active forward channel 1"},
        {4404, "T4 Energy Active forward channel 2"},
        {4408, "T4 Energy Active forward channel 3"},
        {4412, "T4 Energy Active forward channel 4"},
        {4416, "T4 Energy Active forward channel 5"},
        {4420, "T4 Energy Active forward channel 6"},
        {4424, "T4 Energy Active forward channel 7"},
        {4428, "T4 Energy Active forward channel 8"},
        {4401, "T4 Energy Reactive forward channel 1"},
        {4405, "T4 Energy Reactive forward channel 2"},
        {4409, "T4 Energy Reactive forward channel 3"},
        {4413, "T4 Energy Reactive forward channel 4"},
        {4417, "T4 Energy Reactive forward channel 5"},
        {4421, "T4 Energy Reactive forward channel 6"},
        {4425, "T4 Energy Reactive forward channel 7"},
        {4429, "T4 Energy Reactive forward channel 8"},
        {4402, "T4 Energy Active reverse channel 1"},
        {4406, "T4 Energy Active reverse channel 2"},
        {4410, "T4 Energy Active reverse channel 3"},
        {4414, "T4 Energy Active reverse channel 4"},
        {4418, "T4 Energy Active reverse channel 5"},
        {4422, "T4 Energy Active reverse channel 6"},
        {4426, "T4 Energy Active reverse channel 7"},
        {4430, "T4 Energy Active reverse channel 8"},
        {4403, "T4 Energy Reactive reverse channel 1"},
        {4407, "T4 Energy Reactive reverse channel 2"},
        {4411, "T4 Energy Reactive reverse channel 3"},
        {4415, "T4 Energy Reactive reverse channel 4"},
        {4419, "T4 Energy Reactive reverse channel 5"},
        {4423, "T4 Energy Reactive reverse channel 6"},
        {4427, "T4 Energy Reactive reverse channel 7"},
        {4431, "T4 Energy Reactive reverse channel 8"},
};

char *addresss_to_string(int channel_id) {
    for (int i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        if (entries[i].channelId == channel_id) {
            return entries[i].channelDesc;
        }
    }
    return "unknown channel";
}

static char *QualityToString(unsigned int quality) {
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

static json_object *create_master_object(const char *address, const uint16_t port) {
    json_object *master_object = json_object_new_object();
    json_object_object_add(master_object, ADDRESS, json_object_new_string(address));
    json_object_object_add(master_object, PORT, json_object_new_int(port));
    json_object_object_add(master_object, OBJECT_TIMESTAMP, json_object_new_int64(currentTimeMillis()));
    json_object_object_add(master_object, MEASUREMENTS, json_object_new_array());
    return master_object;
}

static void put_measurement(struct json_object *master_object, struct json_object *measurement) {
    struct json_object *ioa;
    json_object_object_get_ex(measurement, OBJECT_ADDRESS, &ioa);
    int ioa_int = json_object_get_int(ioa);

    struct json_object *desc = json_object_new_string(addresss_to_string(ioa_int));
    json_object_object_add(measurement, OBJECT_DESCRIPTION, desc);

    struct json_object *cot_json;
    json_object_object_get_ex(measurement, OBJECT_COT, &cot_json);
    const char *cot = json_object_get_string(cot_json);
//    if(strcmp(cot, CS101_CauseOfTransmission_toString(CS101_COT_INTERROGATED_BY_STATION)) == 0) {
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
//    } else {
//        printf("%s: Ignoring wrong COT %s\n", __func__, cot);
//    }

    if (strcmp(cot, CS101_CauseOfTransmission_toString(CS101_COT_ACTIVATION_TERMINATION)) == 0) {
        CONNECTION_CLOSING_FLAG = true;
        //exit(EXIT_SUCCESS);
    }

}

// M_SP_NA_1: "Single-point information"
static void jsonify_M_SP_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(master_object, measurement);
        SinglePointInformation_destroy((SinglePointInformation) io);
    }
}

/*
// M_SP_TA_1: "Single-point information with time tag"
static void jsonify_M_SP_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_SP_TA_1_getValue((M_SP_TA_1) io)));

        put_measurement(master_object, measurement);
        M_SP_TA_1_destroy((M_SP_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_DP_NA_1: "Double-point information"
static void jsonify_M_DP_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_DP_NA_1_getValue((M_DP_NA_1) io)));

        put_measurement(master_object, measurement);
        M_DP_NA_1_destroy((M_DP_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_DP_TA_1: "Double-point information with time tag"
static void jsonify_M_DP_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_DP_TA_1_getValue((M_DP_TA_1) io)));

        put_measurement(master_object, measurement);
        M_DP_TA_1_destroy((M_DP_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ST_NA_1: "Step position information"
static void jsonify_M_ST_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ST_NA_1_getValue((M_ST_NA_1) io)));

        put_measurement(master_object, measurement);
        M_ST_NA_1_destroy((M_ST_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ST_TA_1: "Step position information with time tag"
static void jsonify_M_ST_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ST_TA_1_getValue((M_ST_TA_1) io)));

        put_measurement(master_object, measurement);
        M_ST_TA_1_destroy((M_ST_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// M_BO_NA_1: "Bitstring of 32 bit"
static void jsonify_M_BO_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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
            json_object_object_add(master_object, DEVICE_ID, json_object_new_string(value));
        }
        else {
            json_object_object_add(master_object, DEVICE_ID, json_object_new_string("NO_IOA_1000_PRESENT"));
        }

        json_object_object_add(measurement, OBJECT_COT,
                               json_object_new_string(CS101_CauseOfTransmission_toString(CS101_ASDU_getCOT(asdu))));

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(master_object, measurement);



        BitString32_destroy((BitString32) io);
    }
}

/*
// M_BO_TA_1: "Bitstring of 32 bit with time tag"
static void jsonify_M_BO_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_BO_TA_1_getValue((M_BO_TA_1) io)));

        put_measurement(master_object, measurement);
        M_BO_TA_1_destroy((M_BO_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ME_NA_1: "Measured value, normalised value"
static void jsonify_M_ME_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_NA_1_getValue((M_ME_NA_1) io)));

        put_measurement(master_object, measurement);
        M_ME_NA_1_destroy((M_ME_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ME_TA_1: "Measured value, normalized value with time tag"
static void jsonify_M_ME_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_TA_1_getValue((M_ME_TA_1) io)));

        put_measurement(master_object, measurement);
        M_ME_TA_1_destroy((M_ME_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// M_ME_NB_1: "Measured value, scaled value"
static void jsonify_M_ME_NB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        char *quality = QualityToString(MeasuredValueShort_getQuality((MeasuredValueShort) io));
        json_object_object_add(measurement, OBJECT_QUALITY, json_object_new_string(quality));
        free(quality);

        put_measurement(master_object, measurement);
        MeasuredValueScaled_destroy((MeasuredValueScaled) io);
    }
}

/*
// M_ME_TB_1: "Measured value, scaled value wit time tag"
static void jsonify_M_ME_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_TB_1_getValue((M_ME_TB_1) io)));

        put_measurement(master_object, measurement);
        M_ME_TB_1_destroy((M_ME_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// M_ME_NC_1: "Measured value, short floating point number"
static void jsonify_M_ME_NC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        MeasuredValueShort_destroy((MeasuredValueShort) io);
    }
}

/*
// M_ME_TC_1: "Measured value, short floating point number with time tag"
static void jsonify_M_ME_TC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_TC_1_getValue((M_ME_TC_1) io)));

        put_measurement(master_object, measurement);
        M_ME_TC_1_destroy((M_ME_TC_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_IT_NA_1: "Integrated totals"
static void jsonify_M_IT_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_IT_NA_1_getValue((M_IT_NA_1) io)));

        put_measurement(master_object, measurement);
        M_IT_NA_1_destroy((M_IT_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_IT_TA_1: "Integrated totals with time tag"
static void jsonify_M_IT_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_IT_TA_1_getValue((M_IT_TA_1) io)));

        put_measurement(master_object, measurement);
        M_IT_TA_1_destroy((M_IT_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TA_1: "Event of protection equipment with time tag"
static void jsonify_M_EP_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TA_1_getValue((M_EP_TA_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TA_1_destroy((M_EP_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TB_1: "Packed start events of protection equipment with time tag"
static void jsonify_M_EP_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TB_1_getValue((M_EP_TB_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TB_1_destroy((M_EP_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TC_1: "Packed output circuit information of protection equipment with time tag"
static void jsonify_M_EP_TC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TC_1_getValue((M_EP_TC_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TC_1_destroy((M_EP_TC_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_PS_NA_1: "Packed single point information with status change detection"
static void jsonify_M_PS_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_PS_NA_1_getValue((M_PS_NA_1) io)));

        put_measurement(master_object, measurement);
        M_PS_NA_1_destroy((M_PS_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ME_ND_1: "Measured value, normalized value without quality descriptor"
static void jsonify_M_ME_ND_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_ND_1_getValue((M_ME_ND_1) io)));

        put_measurement(master_object, measurement);
        M_ME_ND_1_destroy((M_ME_ND_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// M_SP_TB_1: "Single-point information with time tag CP56Time2a"
static void jsonify_M_SP_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        SinglePointWithCP56Time2a_destroy((SinglePointWithCP56Time2a) io);
    }
}

/*
// M_DP_TB_1: "Double-point information with time tag CP56Time2a"
static void jsonify_M_DP_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_DP_TB_1_getValue((M_DP_TB_1) io)));

        put_measurement(master_object, measurement);
        M_DP_TB_1_destroy((M_DP_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ST_TB_1: "Step position information with time tag CP56Time2a"
static void jsonify_M_ST_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ST_TB_1_getValue((M_ST_TB_1) io)));

        put_measurement(master_object, measurement);
        M_ST_TB_1_destroy((M_ST_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_BO_TB_1: "Bitstring of 32 bit with time tag CP56Time2a"
static void jsonify_M_BO_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_BO_TB_1_getValue((M_BO_TB_1) io)));

        put_measurement(master_object, measurement);
        M_BO_TB_1_destroy((M_BO_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ME_TD_1: "Measured value, normalised value with time tag CP56Time2a"
static void jsonify_M_ME_TD_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_TD_1_getValue((M_ME_TD_1) io)));

        put_measurement(master_object, measurement);
        M_ME_TD_1_destroy((M_ME_TD_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_ME_TE_1: "Measured value, scaled value with time tag CP56Time2a"
static void jsonify_M_ME_TE_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_ME_TE_1_getValue((M_ME_TE_1) io)));

        put_measurement(master_object, measurement);
        M_ME_TE_1_destroy((M_ME_TE_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// M_ME_TF_1: "Measured value, short floating point number with time tag CP56Time2a"
static void jsonify_M_ME_TF_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        MeasuredValueShortWithCP56Time2a_destroy((MeasuredValueShortWithCP56Time2a) io);
    }
}

/*
// M_IT_TB_1: "Integrated totals with time tag CP56Time2a"
static void jsonify_M_IT_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_IT_TB_1_getValue((M_IT_TB_1) io)));

        put_measurement(master_object, measurement);
        M_IT_TB_1_destroy((M_IT_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TD_1: "Event of protection equipment with time tag CP56Time2a"
static void jsonify_M_EP_TD_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TD_1_getValue((M_EP_TD_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TD_1_destroy((M_EP_TD_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TE_1: "Packed start events of protection equipment with time tag CP56Time2a"
static void jsonify_M_EP_TE_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TE_1_getValue((M_EP_TE_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TE_1_destroy((M_EP_TE_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EP_TF_1: "Packed output circuit information of protection equipment with time tag CP56Time2a"
static void jsonify_M_EP_TF_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EP_TF_1_getValue((M_EP_TF_1) io)));

        put_measurement(master_object, measurement);
        M_EP_TF_1_destroy((M_EP_TF_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SC_NA_1: "Single command"
static void jsonify_C_SC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SC_NA_1_getValue((C_SC_NA_1) io)));

        put_measurement(master_object, measurement);
        C_SC_NA_1_destroy((C_SC_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_DC_NA_1: "Double command"
static void jsonify_C_DC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_DC_NA_1_getValue((C_DC_NA_1) io)));

        put_measurement(master_object, measurement);
        C_DC_NA_1_destroy((C_DC_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_RC_NA_1: "Regulating step command"
static void jsonify_C_RC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_RC_NA_1_getValue((C_RC_NA_1) io)));

        put_measurement(master_object, measurement);
        C_RC_NA_1_destroy((C_RC_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_NA_1: "Set-point Command, normalised value"
static void jsonify_C_SE_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_NA_1_getValue((C_SE_NA_1) io)));

        put_measurement(master_object, measurement);
        C_SE_NA_1_destroy((C_SE_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_NB_1: "Set-point Command, scaled value"
static void jsonify_C_SE_NB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_NB_1_getValue((C_SE_NB_1) io)));

        put_measurement(master_object, measurement);
        C_SE_NB_1_destroy((C_SE_NB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_NC_1: "Set-point Command, short floating point number"
static void jsonify_C_SE_NC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_NC_1_getValue((C_SE_NC_1) io)));

        put_measurement(master_object, measurement);
        C_SE_NC_1_destroy((C_SE_NC_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_BO_NA_1: "Bitstring 32 bit command"
static void jsonify_C_BO_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_BO_NA_1_getValue((C_BO_NA_1) io)));

        put_measurement(master_object, measurement);
        C_BO_NA_1_destroy((C_BO_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SC_TA_1: "Single command with time tag CP56Time2a"
static void jsonify_C_SC_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SC_TA_1_getValue((C_SC_TA_1) io)));

        put_measurement(master_object, measurement);
        C_SC_TA_1_destroy((C_SC_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_DC_TA_1: "Double command with time tag CP56Time2a"
static void jsonify_C_DC_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_DC_TA_1_getValue((C_DC_TA_1) io)));

        put_measurement(master_object, measurement);
        C_DC_TA_1_destroy((C_DC_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_RC_TA_1: "Regulating step command with time tag CP56Time2a"
static void jsonify_C_RC_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_RC_TA_1_getValue((C_RC_TA_1) io)));

        put_measurement(master_object, measurement);
        C_RC_TA_1_destroy((C_RC_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_TA_1: "Measured value, normalised value command with time tag CP56Time2a"
static void jsonify_C_SE_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_TA_1_getValue((C_SE_TA_1) io)));

        put_measurement(master_object, measurement);
        C_SE_TA_1_destroy((C_SE_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_TB_1: "Measured value, scaled value command with time tag CP56Time2a"
static void jsonify_C_SE_TB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_TB_1_getValue((C_SE_TB_1) io)));

        put_measurement(master_object, measurement);
        C_SE_TB_1_destroy((C_SE_TB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_SE_TC_1: "Measured value, short floating point number command with time tag CP56Time2a"
static void jsonify_C_SE_TC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_SE_TC_1_getValue((C_SE_TC_1) io)));

        put_measurement(master_object, measurement);
        C_SE_TC_1_destroy((C_SE_TC_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_BO_TA_1: "Bitstring of 32 bit command with time tag CP56Time2a"
static void jsonify_C_BO_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_BO_TA_1_getValue((C_BO_TA_1) io)));

        put_measurement(master_object, measurement);
        C_BO_TA_1_destroy((C_BO_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// M_EI_NA_1: "End of Initialisation"
static void jsonify_M_EI_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(M_EI_NA_1_getValue((M_EI_NA_1) io)));

        put_measurement(master_object, measurement);
        M_EI_NA_1_destroy((M_EI_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// C_IC_NA_1: "Interrogation command"
static void jsonify_C_IC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        InterrogationCommand_destroy((InterrogationCommand) io);
    }
}

// C_CI_NA_1: "Counter interrogation command"
static void jsonify_C_CI_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        CounterInterrogationCommand_destroy((CounterInterrogationCommand) io);
    }
}

/*
// C_RD_NA_1: "Read command"
static void jsonify_C_RD_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_RD_NA_1_getValue((C_RD_NA_1) io)));

        put_measurement(master_object, measurement);
        C_RD_NA_1_destroy((C_RD_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

// C_CS_NA_1: "Clock synchronisation command"
static void jsonify_C_CS_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
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

        put_measurement(master_object, measurement);
        ClockSynchronizationCommand_destroy((ClockSynchronizationCommand) io);
    }
}

/*
// C_TS_NA_1: "Test command"
static void jsonify_C_TS_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_TS_NA_1_getValue((C_TS_NA_1) io)));

        put_measurement(master_object, measurement);
        C_TS_NA_1_destroy((C_TS_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_RP_NA_1: "Reset process command"
static void jsonify_C_RP_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_RP_NA_1_getValue((C_RP_NA_1) io)));

        put_measurement(master_object, measurement);
        C_RP_NA_1_destroy((C_RP_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_CD_NA_1: "Delay acquisition command"
static void jsonify_C_CD_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_CD_NA_1_getValue((C_CD_NA_1) io)));

        put_measurement(master_object, measurement);
        C_CD_NA_1_destroy((C_CD_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// C_TS_TA_1: "Test command with time tag CP56Time2a"
static void jsonify_C_TS_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(C_TS_TA_1_getValue((C_TS_TA_1) io)));

        put_measurement(master_object, measurement);
        C_TS_TA_1_destroy((C_TS_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// P_ME_NA_1: "Parameter of measured values, normalized value"
static void jsonify_P_ME_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(P_ME_NA_1_getValue((P_ME_NA_1) io)));

        put_measurement(master_object, measurement);
        P_ME_NA_1_destroy((P_ME_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// P_ME_NB_1: "Parameter of measured values, scaled value"
static void jsonify_P_ME_NB_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(P_ME_NB_1_getValue((P_ME_NB_1) io)));

        put_measurement(master_object, measurement);
        P_ME_NB_1_destroy((P_ME_NB_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// P_ME_NC_1: "Parameter of measured values, short floating point number"
static void jsonify_P_ME_NC_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(P_ME_NC_1_getValue((P_ME_NC_1) io)));

        put_measurement(master_object, measurement);
        P_ME_NC_1_destroy((P_ME_NC_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// P_AC_NA_1: "Parameter activation"
static void jsonify_P_AC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(P_AC_NA_1_getValue((P_AC_NA_1) io)));

        put_measurement(master_object, measurement);
        P_AC_NA_1_destroy((P_AC_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_FR_NA_1: "File ready"
static void jsonify_F_FR_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_FR_NA_1_getValue((F_FR_NA_1) io)));

        put_measurement(master_object, measurement);
        F_FR_NA_1_destroy((F_FR_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_SR_NA_1: "Section ready"
static void jsonify_F_SR_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_SR_NA_1_getValue((F_SR_NA_1) io)));

        put_measurement(master_object, measurement);
        F_SR_NA_1_destroy((F_SR_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_SC_NA_1: "Call directory, select file, call file, call section"
static void jsonify_F_SC_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_SC_NA_1_getValue((F_SC_NA_1) io)));

        put_measurement(master_object, measurement);
        F_SC_NA_1_destroy((F_SC_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_LS_NA_1: "Last section, last segment"
static void jsonify_F_LS_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_LS_NA_1_getValue((F_LS_NA_1) io)));

        put_measurement(master_object, measurement);
        F_LS_NA_1_destroy((F_LS_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_FA_NA_1: "ACK file, ACK section"
static void jsonify_F_FA_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_FA_NA_1_getValue((F_FA_NA_1) io)));

        put_measurement(master_object, measurement);
        F_FA_NA_1_destroy((F_FA_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_SG_NA_1: "Segment"
static void jsonify_F_SG_NA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_SG_NA_1_getValue((F_SG_NA_1) io)));

        put_measurement(master_object, measurement);
        F_SG_NA_1_destroy((F_SG_NA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/

/*
// F_DR_TA_1: "Directory"
static void jsonify_F_DR_TA_1(struct sCS101_ASDU *asdu, struct json_object *master_object) {
    for (int i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
        json_object *measurement = json_object_new_object();
        InformationObject io = CS101_ASDU_getElement(asdu, i);

        json_object_object_add(measurement, OBJECT_TYPE, json_object_new_string(TypeID_toString(CS101_ASDU_getTypeID(asdu))));
        json_object_object_add(measurement, OBJECT_ADDRESS, json_object_new_int(InformationObject_getObjectAddress(io)));
        json_object_object_add(measurement, OBJECT_VALUE, json_object_new_int(F_DR_TA_1_getValue((F_DR_TA_1) io)));

        put_measurement(master_object, measurement);
        F_DR_TA_1_destroy((F_DR_TA_1)io);
    }
    fprintf(stderr, "%s: not implemented yet!\n", __func__);
    exit(EXIT_FAILURE);
}
*/


/* Callback handler to log sent or received messages (optional) */
static void
rawMessageHandler(void *parameter, uint8_t *msg, int msgSize, bool sent) {
    (void) parameter;
    if (sent)
        printf("SEND: ");
    else
        printf("RCVD: ");

    int i;
    for (i = 0; i < msgSize; i++) {
        printf("%02x ", msg[i]);
    }

    printf("\n");
}

/* Connection event handler */
static void
connectionHandler(void *parameter, CS104_Connection connection, CS104_ConnectionEvent event) {
    switch (event) {
        case CS104_CONNECTION_OPENED:
            printf("Connection established\n");
            CS104_Connection_sendStartDT(connection);
            break;
        case CS104_CONNECTION_CLOSED:
            printf("Connection closed\n");
            //exit(EXIT_SUCCESS);
            break;
        case CS104_CONNECTION_STARTDT_CON_RECEIVED:
            printf("Received STARTDT_CON\n");
            CS104_Connection_sendInterrogationCommand(connection, CS101_COT_ACTIVATION, 1, IEC60870_QOI_STATION);
//            CS104_Connection_sendCounterInterrogationCommand(connection, IEC60870_QCC_RQT_GENERAL, 1, IEC60870_QOI_STATION); // TODO: Verify
            break;
        case CS104_CONNECTION_STOPDT_CON_RECEIVED:
            printf("Received STOPDT_CON\n");
            CS104_Connection_destroy(connection);
            break;
        default:
            fprintf(stderr, "Received unknown event %d\n", event);
//            exit(EXIT_FAILURE);
    }
}

void handle_M_ME_TE_1(struct sCS101_ASDU *asdu) {
    printf("  measured scaled values with CP56Time2a timestamp:\n");

    int i;

    for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {

        MeasuredValueScaledWithCP56Time2a io =
                (MeasuredValueScaledWithCP56Time2a) CS101_ASDU_getElement(asdu, i);

        printf("    IOA: %i value: %i\n",
               InformationObject_getObjectAddress((InformationObject) io),
               MeasuredValueScaled_getValue((MeasuredValueScaled) io)
        );

        MeasuredValueScaledWithCP56Time2a_destroy(io);
    }
}

void handle_M_ME_TB_1(struct sCS101_ASDU *asdu) {
    printf("  scaled measured value (-32768 ... +32767) with CP24Time2a:\n");

    int i;

    for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {

        MeasuredValueScaledWithCP24Time2a io =
                (MeasuredValueScaledWithCP24Time2a) CS101_ASDU_getElement(asdu, i);

        char *time;
        printf("    IOA: %i value: %d Timestamp: ",
               InformationObject_getObjectAddress((InformationObject) io),
               MeasuredValueScaled_getValue((MeasuredValueScaled) io)
        );
        CP24Time2a timestamp = MeasuredValueScaledWithCP24Time2a_getTimestamp(io);
        printf("%d:%d.%d\n",
               CP24Time2a_getMinute(timestamp),
               CP24Time2a_getSecond(timestamp),
               CP24Time2a_getMillisecond(timestamp)
        );
        MeasuredValueScaledWithCP24Time2a_destroy(io);
    }
}

/*
 * CS101_ASDUReceivedHandler implementation
 *
 * For CS104 the address parameter has to be ignored
 */
static bool
asduReceivedHandler(void *parameter, int address, CS101_ASDU asdu) {
    json_object *master_object = parameter;
    const int type = CS101_ASDU_getTypeID(asdu);
    const char *typeStr = TypeID_toString(type);
    const int cot = CS101_ASDU_getCOT(asdu);
    printf("RECVD ASDU type: %s(%i) elements: %i cot %s(%i)\n",
           typeStr,
           type,
           CS101_ASDU_getNumberOfElements(asdu),
           CS101_CauseOfTransmission_toString(cot),
           cot);
    switch (type) {
        case M_SP_NA_1: // "Single-point information"
            jsonify_M_SP_NA_1(asdu, master_object);
            break;
//        case M_SP_TA_1: // "Single-point information with time tag"
//            jsonify_M_SP_TA_1(asdu, master_object);
//            break;
//        case M_DP_NA_1: // "Double-point information"
//            jsonify_M_DP_NA_1(asdu, master_object);
//            break;
//        case M_DP_TA_1: // "Double-point information with time tag"
//            jsonify_M_DP_TA_1(asdu, master_object);
//            break;
//        case M_ST_NA_1: // "Step position information"
//            jsonify_M_ST_NA_1(asdu, master_object);
//            break;
//        case M_ST_TA_1: // "Step position information with time tag"
//            jsonify_M_ST_TA_1(asdu, master_object);
//            break;
        case M_BO_NA_1: // "Bitstring of 32 bit"
            jsonify_M_BO_NA_1(asdu, master_object);
            break;
//        case M_BO_TA_1: // "Bitstring of 32 bit with time tag"
//            jsonify_M_BO_TA_1(asdu, master_object);
//            break;
//        case M_ME_NA_1: // "Measured value, normalised value"
//            jsonify_M_ME_NA_1(asdu, master_object);
//            break;
//        case M_ME_TA_1: // "Measured value, normalized value with time tag"
//            jsonify_M_ME_TA_1(asdu, master_object);
//            break;
        case M_ME_NB_1: // "Measured value, scaled value"
            jsonify_M_ME_NB_1(asdu, master_object);
            break;
//        case M_ME_TB_1: // "Measured value, scaled value wit time tag"
//            jsonify_M_ME_TB_1(asdu, master_object);
//            break;
        case M_ME_NC_1: // "Measured value, short floating point number"
            jsonify_M_ME_NC_1(asdu, master_object);
            break;
//        case M_ME_TC_1: // "Measured value, short floating point number with time tag"
//            jsonify_M_ME_TC_1(asdu, master_object);
//            break;
//        case M_IT_NA_1: // "Integrated totals"
//            jsonify_M_IT_NA_1(asdu, master_object);
//            break;
//        case M_IT_TA_1: // "Integrated totals with time tag"
//            jsonify_M_IT_TA_1(asdu, master_object);
//            break;
//        case M_EP_TA_1: // "Event of protection equipment with time tag"
//            jsonify_M_EP_TA_1(asdu, master_object);
//            break;
//        case M_EP_TB_1: // "Packed start events of protection equipment with time tag"
//            jsonify_M_EP_TB_1(asdu, master_object);
//            break;
//        case M_EP_TC_1: // "Packed output circuit information of protection equipment with time tag"
//            jsonify_M_EP_TC_1(asdu, master_object);
//            break;
//        case M_PS_NA_1: // "Packed single point information with status change detection"
//            jsonify_M_PS_NA_1(asdu, master_object);
//            break;
//        case M_ME_ND_1: // "Measured value, normalized value without quality descriptor"
//            jsonify_M_ME_ND_1(asdu, master_object);
//            break;
        case M_SP_TB_1: // "Single-point information with time tag CP56Time2a"
            jsonify_M_SP_TB_1(asdu, master_object);
            break;
//        case M_DP_TB_1: // "Double-point information with time tag CP56Time2a"
//            jsonify_M_DP_TB_1(asdu, master_object);
//            break;
//        case M_ST_TB_1: // "Step position information with time tag CP56Time2a"
//            jsonify_M_ST_TB_1(asdu, master_object);
//            break;
//        case M_BO_TB_1: // "Bitstring of 32 bit with time tag CP56Time2a"
//            jsonify_M_BO_TB_1(asdu, master_object);
//            break;
//        case M_ME_TD_1: // "Measured value, normalised value with time tag CP56Time2a"
//            jsonify_M_ME_TD_1(asdu, master_object);
//            break;
//        case M_ME_TE_1: // "Measured value, scaled value with time tag CP56Time2a"
//            jsonify_M_ME_TE_1(asdu, master_object);
//            break;
        case M_ME_TF_1: // "Measured value, short floating point number with time tag CP56Time2a"
            jsonify_M_ME_TF_1(asdu, master_object);
            break;
//        case M_IT_TB_1: // "Integrated totals with time tag CP56Time2a"
//            jsonify_M_IT_TB_1(asdu, master_object);
//            break;
//        case M_EP_TD_1: // "Event of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TD_1(asdu, master_object);
//            break;
//        case M_EP_TE_1: // "Packed start events of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TE_1(asdu, master_object);
//            break;
//        case M_EP_TF_1: // "Packed output circuit information of protection equipment with time tag CP56Time2a"
//            jsonify_M_EP_TF_1(asdu, master_object);
//            break;
//        case C_SC_NA_1: // "Single command"
//            jsonify_C_SC_NA_1(asdu, master_object);
//            break;
//        case C_DC_NA_1: // "Double command"
//            jsonify_C_DC_NA_1(asdu, master_object);
//            break;
//        case C_RC_NA_1: // "Regulating step command"
//            jsonify_C_RC_NA_1(asdu, master_object);
//            break;
//        case C_SE_NA_1: // "Set-point Command, normalised value"
//            jsonify_C_SE_NA_1(asdu, master_object);
//            break;
//        case C_SE_NB_1: // "Set-point Command, scaled value"
//            jsonify_C_SE_NB_1(asdu, master_object);
//            break;
//        case C_SE_NC_1: // "Set-point Command, short floating point number"
//            jsonify_C_SE_NC_1(asdu, master_object);
//            break;
//        case C_BO_NA_1: // "Bitstring 32 bit command"
//            jsonify_C_BO_NA_1(asdu, master_object);
//            break;
//        case C_SC_TA_1: // "Single command with time tag CP56Time2a"
//            jsonify_C_SC_TA_1(asdu, master_object);
//            break;
//        case C_DC_TA_1: // "Double command with time tag CP56Time2a"
//            jsonify_C_DC_TA_1(asdu, master_object);
//            break;
//        case C_RC_TA_1: // "Regulating step command with time tag CP56Time2a"
//            jsonify_C_RC_TA_1(asdu, master_object);
//            break;
//        case C_SE_TA_1: // "Measured value, normalised value command with time tag CP56Time2a"
//            jsonify_C_SE_TA_1(asdu, master_object);
//            break;
//        case C_SE_TB_1: // "Measured value, scaled value command with time tag CP56Time2a"
//            jsonify_C_SE_TB_1(asdu, master_object);
//            break;
//        case C_SE_TC_1: // "Measured value, short floating point number command with time tag CP56Time2a"
//            jsonify_C_SE_TC_1(asdu, master_object);
//            break;
//        case C_BO_TA_1: // "Bitstring of 32 bit command with time tag CP56Time2a"
//            jsonify_C_BO_TA_1(asdu, master_object);
//            break;
//        case M_EI_NA_1: // "End of Initialisation"
//            jsonify_M_EI_NA_1(asdu, master_object);
//            break;
        case C_IC_NA_1: // "Interrogation command"
            jsonify_C_IC_NA_1(asdu, master_object);
            break;
        case C_CI_NA_1: // "Counter interrogation command"
            jsonify_C_CI_NA_1(asdu, master_object);
            break;
//        case C_RD_NA_1: // "Read command"
//            jsonify_C_RD_NA_1(asdu, master_object);
//            break;
        case C_CS_NA_1: // "Clock synchronisation command"
            jsonify_C_CS_NA_1(asdu, master_object);
            break;
//        case C_TS_NA_1: // "Test command"
//            jsonify_C_TS_NA_1(asdu, master_object);
//            break;
//        case C_RP_NA_1: // "Reset process command"
//            jsonify_C_RP_NA_1(asdu, master_object);
//            break;
//        case C_CD_NA_1: // "Delay acquisition command"
//            jsonify_C_CD_NA_1(asdu, master_object);
//            break;
//        case C_TS_TA_1: // "Test command with time tag CP56Time2a"
//            jsonify_C_TS_TA_1(asdu, master_object);
//            break;
//        case P_ME_NA_1: // "Parameter of measured values, normalized value"
//            jsonify_P_ME_NA_1(asdu, master_object);
//            break;
//        case P_ME_NB_1: // "Parameter of measured values, scaled value"
//            jsonify_P_ME_NB_1(asdu, master_object);
//            break;
//        case P_ME_NC_1: // "Parameter of measured values, short floating point number"
//            jsonify_P_ME_NC_1(asdu, master_object);
//            break;
//        case P_AC_NA_1: // "Parameter activation"
//            jsonify_P_AC_NA_1(asdu, master_object);
//            break;
//        case F_FR_NA_1: // "File ready"
//            jsonify_F_FR_NA_1(asdu, master_object);
//            break;
//        case F_SR_NA_1: // "Section ready"
//            jsonify_F_SR_NA_1(asdu, master_object);
//            break;
//        case F_SC_NA_1: // "Call directory, select file, call file, call section"
//            jsonify_F_SC_NA_1(asdu, master_object);
//            break;
//        case F_LS_NA_1: // "Last section, last segment"
//            jsonify_F_LS_NA_1(asdu, master_object);
//            break;
//        case F_FA_NA_1: // "ACK file, ACK section"
//            jsonify_F_FA_NA_1(asdu, master_object);
//            break;
//        case F_SG_NA_1: // "Segment"
//            jsonify_F_SG_NA_1(asdu, master_object);
//            break;
//        case F_DR_TA_1: // "Directory"
//            jsonify_F_DR_TA_1(asdu, master_object);
//            break;
        default:
            fprintf(stderr, "Got not implemented yet ASDU type %s(%d)\n", typeStr, type);
            //exit(1);
    }
    return true;
}

static int
iec_104_fetch(struct lua_State *L) {

    const char *ip;
    uint16_t port;

    if (lua_gettop(L) < 2)
    		luaL_error(L, "Usage: fetch(host: string, port: number)");

    ip = lua_tostring(L, 1);
    port = lua_tointeger(L, 2);

    //printf("Connecting to: %s:%i\n", ip, port);
    CS104_Connection con = CS104_Connection_create(ip, port);
    json_object *master_object = create_master_object(ip, port);
    CS104_Connection_setConnectionHandler(con, connectionHandler, master_object);
    CS104_Connection_setASDUReceivedHandler(con, asduReceivedHandler, master_object);

    /* uncomment to log messages */
    //CS104_Connection_setRawMessageHandler(con, rawMessageHandler, NULL);
    if(CS104_Connection_connect(con)) {
        long int time_start;
        long int time_current;
        time_start = time(NULL);
        while (!CONNECTION_CLOSING_FLAG) {
            Thread_sleep(100);
            time_current = time(NULL);
            if (time_current - time_start > 15) {
                break;
            }
        }
        CS104_Connection_sendStopDT(con);
        CONNECTION_CLOSING_FLAG = false;
    }
    else {

    }

    const char *json_string = json_object_to_json_string(master_object);
    lua_pushstring(L, json_string);

    free((char *)json_string);
    //json_object_put(master_object);

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
