/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * Copyright
 * File:   PromotionAnalyzer.cpp
 * Author: 이해성, geneel@me.com
 *
 * Created on 2025년 7월 1일 (화), 오전 12:35

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */


#define MIN_TOKENS  128
#define TIME_SLOT_SIZE 2048 //FFT-PSD와 DWT-TimeMax를 실행한 결과값들의 갯수
#define TEXT_SIZE 6000 //한글을 기준으로 한 글자수
#define EMB_SIZE 768 //google-vertex AI에서 TextEmbedding을 실행한 결과 벡터의 차원 갯수
#define STR_SIZE 2048
#define NAME_SIZE 256
#define READ_BUF_SIZE 64
#define CHUNK_SIZE 10
#define ONE_DAY_IN_SEC 86400
#define ONE_DAY_IN_DAY 1
#define ONE_WEEK_IN_DAY 7
#define ONE_MONTH_IN_DAY 30
#define ONE_YEAR_IN_DAY 365
//#define MANUAL_DEBUG 1

//  For Signal Processing : Start
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define KLEN 32  /* 고정: 32-탭 가우시안 커널 */
#define DWT_KLEN 12 /* 고정: 12-탭 웨이블릿 커널 */
#define SIGMA 5.33
//  For Signal Processing : End

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
//#include <cstring>
#include <rapidjson/document.h>
#include <mongoc.h>
#include <bson.h>
#include <mysql.h>
#include <my_global.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>


using namespace rapidjson;
using namespace std;


static char caResult[TEXT_SIZE*20];

typedef struct
{
    char _id[READ_BUF_SIZE] = {0};
    char PromotionID[READ_BUF_SIZE] = {0};
    int64_t RegisteredDate = 0, StartDate = 0, EndDate = 0;
    int ArtIDsNum = 0, QueryKeywordsNum = 0;
    char ArticleIDs[MIN_TOKENS][NAME_SIZE] = {0};
    char QueryKeywords[MIN_TOKENS][NAME_SIZE] = {0};
    char ClassAxis[TEXT_SIZE] = {0};
    char WriterAxis[TEXT_SIZE] = {0};
    int TotalClassNum = 0, TotalWriterNum = 0;
    double NormalClass[EMB_SIZE] = {0};
    double NormalWriter[EMB_SIZE] = {0};
    double ArticleEmbedding[EMB_SIZE] = {0};
    double QueryEmbedding[EMB_SIZE] = {0};
} PromotionDocu;

typedef struct
{
    char cid[READ_BUF_SIZE*2];   //방문자코드
    char created_at[READ_BUF_SIZE*2];    //접속시각
    char properties_value[STR_SIZE];  //홈페이지 URL
    char properties_referrer[STR_SIZE];   //레퍼러 URL
    
    time_t EventTime;   //해당 이벤트가 발생한 시각의 로컬 타임
    char ART_ID[READ_BUF_SIZE]; //해당 기사의 아이디
    char Query[READ_BUF_SIZE];  //레퍼러에 입력한 검색어

    //properties_value[]에서 Key가 id_art인 경우의 Value와 Path에 read가 있는 경우 그 다음 Path가 ART_ID이다.
    //properties_value[]와 properties_referrer[]에서 Key가 q, query, search인 경우의 Value가 검색어이다.
    //Events 컬렉션에서 value와 properties_value는 값이 완전히 동일하다.
} EventDatum;


time_t MakeLocalTime_t(int YYYY, int MM, int DD, int hh, int mi, int ss)
{
	struct tm st_tm;

	st_tm.tm_year = YYYY - 1900;
	st_tm.tm_mon =  MM - 1;
	st_tm.tm_mday = DD;
	st_tm.tm_hour = hh;
	st_tm.tm_min =  mi;
	st_tm.tm_sec =  ss;

	return mktime( &st_tm );
}

time_t MakeUTCTime_t(int YYYY, int MM, int DD, int hh, int mi, int ss)
{
	struct tm st_tm;

	st_tm.tm_year = YYYY - 1900;
	st_tm.tm_mon =  MM - 1;
	st_tm.tm_mday = DD;
	st_tm.tm_hour = hh;
	st_tm.tm_min =  mi;
	st_tm.tm_sec =  ss;

        return timegm( &st_tm );    //Unix 계열
//	return _mkgmtime( &st_tm ); //Windows 계열
}

time_t DatetimeStrToLocalTime(char* caDateTimeStr)   //For MySQL Datetime Obj
{   //DatetimeStr Example : 2023-10-08 04:02:46
    int i, iLength;
    int YYYY, MM, DD, hh, mi, ss;
    char caYYYY[5], caMM[3], caDD[3], cahh[3], cami[3], cass[3];
    
    iLength = strlen(caDateTimeStr);
    if(iLength != 19)
    {
        return MakeLocalTime_t(1, 1, 1, 1, 1, 1);
    }
    
    for(i=0; i<4; i++)
    {
        caYYYY[i] = caDateTimeStr[i];
    }
    caYYYY[4] = '\0';
    
    for(i=0; i<2; i++)
    {
        caMM[i] = caDateTimeStr[i+5];
    }
    caMM[2] = '\0';

    for(i=0; i<2; i++)
    {
        caDD[i] = caDateTimeStr[i+8];
    }
    caDD[2] = '\0';

    for(i=0; i<2; i++)
    {
        cahh[i] = caDateTimeStr[i+11];
    }
    cahh[2] = '\0';

    for(i=0; i<2; i++)
    {
        cami[i] = caDateTimeStr[i+14];
    }
    cami[2] = '\0';

    for(i=0; i<2; i++)
    {
        cass[i] = caDateTimeStr[i+17];
    }
    cass[2] = '\0';
    
    return MakeLocalTime_t(atoi(caYYYY), atoi(caMM), atoi(caDD), atoi(cahh), atoi(cami), atoi(cass));
}

time_t ISO8601StrToLocalTime(char* caISO8601Str)  //For ISO8601 Type String : BigData Svr에서 사용중
{   //ISO8601Str Example : 2023-04-03T16:04:45.747Z
    int i, iLength;
    int YYYY, MM, DD, hh, mi, ss;
    char caYYYY[5], caMM[3], caDD[3], cahh[3], cami[3], cass[3];
    
    iLength = strlen(caISO8601Str);
    if(iLength < 19)
    {
        return MakeLocalTime_t(1, 1, 1, 1, 1, 1);
    }

    for(i=0; i<4; i++)
    {
        caYYYY[i] = caISO8601Str[i];
    }
    caYYYY[4] = '\0';
    
    for(i=0; i<2; i++)
    {
        caMM[i] = caISO8601Str[i+5];
    }
    caMM[2] = '\0';

    for(i=0; i<2; i++)
    {
        caDD[i] = caISO8601Str[i+8];
    }
    caDD[2] = '\0';

    for(i=0; i<2; i++)
    {
        cahh[i] = caISO8601Str[i+11];
    }
    cahh[2] = '\0';

    for(i=0; i<2; i++)
    {
        cami[i] = caISO8601Str[i+14];
    }
    cami[2] = '\0';

    for(i=0; i<2; i++)
    {
        cass[i] = caISO8601Str[i+17];
    }
    cass[2] = '\0';
    
    return MakeLocalTime_t(atoi(caYYYY), atoi(caMM), atoi(caDD), atoi(cahh), atoi(cami), atoi(cass));    
}

time_t DatetimeStr2ToLocalTime(char* caDateTimeStr)   //For MySQL Datetime Obj
{   //DatetimeStr Example : Mar 20 2018 05:30:05:000PM, Mar 16 2021 09:16AM
    int iLength;
    int i;
    int YYYY, MM, DD, hh, mi, ss;
    char caYYYY[5], caMM[4], caDD[3], cahh[3], cami[3], cass[3];
    
    iLength = strlen(caDateTimeStr);
    if((iLength != 26)&&(iLength != 19))
    {
        return MakeLocalTime_t(1, 1, 1, 1, 1, 1);
    }
    
    for(i=0; i<4; i++)
    {
        caYYYY[i] = caDateTimeStr[i+7];
    }
    caYYYY[4] = '\0';
    
    for(i=0; i<3; i++)
    {
        caMM[i] = caDateTimeStr[i];
    }
    caMM[3] = '\0';

    for(i=0; i<2; i++)
    {
        caDD[i] = caDateTimeStr[i+4];
    }
    caDD[2] = '\0';

    if(iLength > 19)
    {
        if(caDateTimeStr[24] == 'A')
        {
            for(i=0; i<2; i++)
            {
                cahh[i] = caDateTimeStr[i+12];
            }
            cahh[2] = '\0';            
        }
        else
        {
            for(i=0; i<2; i++)
            {
                cahh[i] = caDateTimeStr[i+12];
            }
            cahh[2] = '\0';            
            int iTmp = atoi(cahh);
            iTmp += 12;
            snprintf(cahh, sizeof(cahh)-1, "%d", iTmp);
        }
    }
    else
    {
        if(caDateTimeStr[17] == 'A')
        {
            for(i=0; i<2; i++)
            {
                cahh[i] = caDateTimeStr[i+12];
            }
            cahh[2] = '\0';            
        }
        else
        {
            for(i=0; i<2; i++)
            {
                cahh[i] = caDateTimeStr[i+12];
            }
            cahh[2] = '\0';            
            int iTmp = atoi(cahh);
            iTmp += 12;
            snprintf(cahh, sizeof(cahh)-1, "%d", iTmp);
        }
    }

    for(i=0; i<2; i++)
    {
        cami[i] = caDateTimeStr[i+15];
    }
    cami[2] = '\0';

    if(iLength > 19)
    {
        for(i=0; i<2; i++)
        {
            cass[i] = caDateTimeStr[i+18];
        }
        cass[2] = '\0';        
    }
    else
    {
        strcpy(cass, "00");
    }
    
    return MakeLocalTime_t(atoi(caYYYY), atoi(caMM), atoi(caDD), atoi(cahh), atoi(cami), atoi(cass));
}

int GetCurrLocalTime(char *caCurrLocalTime, bool bSpecial)
{   //ISO8601Str Example : 2023-04-03T16:04:45.747Z
    time_t timer;
    struct tm* t;
    timer = time(NULL);   // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
    t = localtime(&timer);   // 포맷팅을 위해 구조체에 넣기

    if(bSpecial)
    {
        sprintf(caCurrLocalTime, "%04d%02d%02d%02d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
    else
    {
        sprintf(caCurrLocalTime, "%04d-%02d-%02dT%02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
            
    return 0;       
}

int GetTimeStr(time_t InputTime, char *caTimeStr, bool bSpecial)
{   //ISO8601Str Example : 2023-04-03T16:04:45.747Z
    time_t timer;
    struct tm* t;
    
//    timer = time(&InputTime);
    timer = InputTime;
    t = localtime(&timer);   // 포맷팅을 위해 구조체에 넣기

    if(bSpecial)
    {
        sprintf(caTimeStr, "%04d%02d%02d%02d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
    else
    {
        sprintf(caTimeStr, "%04d-%02d-%02dT%02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);    
    }
            
    return 0;       
}


void sanitize_shell_input(char *str) 
{
    // 메타문자 목록 정의
    const char *SHELL_META_CHARS = "|&;<>()$`\\\"' \t\n*?[#~=%!{}";
    for (int i = 0; str[i] != '\0'; ++i) {
        // 메타문자 또는 제어문자일 경우 스페이스로 치환
        if (strchr(SHELL_META_CHARS, str[i]) || iscntrl((unsigned char)str[i])) {
            str[i] = ' ';
        }
    }
}

void sanitize_slash(char *str) 
{
    // 메타문자 목록 정의
    const char *SHELL_META_CHARS = "/";
    for (int i = 0; str[i] != '\0'; ++i) {
        // 메타문자 또는 제어문자일 경우 스페이스로 치환
        if (strchr(SHELL_META_CHARS, str[i]) || iscntrl((unsigned char)str[i])) {
            str[i] = ' ';
        }
    }
}

void keep_digits(char *str) {
    char *src = str;
    char *dst = str;

    while (*src) {
        if (isdigit((unsigned char)*src)) {
            *dst++ = *src;   // 숫자면 복사
        }
        src++;
    }
    *dst = '\0';  // 문자열 끝에 NULL 추가
}

// URL decoding (간단히 공백(%20)만 처리, 필요시 확장 가능)
void url_decode(char *src, char *dest) {
    char *p = src;
    char code[3] = {0};
    while (*p) {
        if (*p == '%' && p[1] && p[2]) {
            strncpy(code, p + 1, 2);
            *dest++ = (char) strtol(code, NULL, 16);
            p += 3;
        } else if (*p == '+') {
            *dest++ = ' ';
            p++;
        } else {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}

/**
 * URL에서 주어진 key(예: "q=", "query=")를 찾아 검색어 추출
 * url: 전체 URL 문자열
 * key: 찾고자 하는 key 문자열
 * 반환값: malloc으로 동적할당된 검색어 문자열 (사용 후 free 필요)
 */
char* extract_value(char *url, const char *key) {
    if (!url || !key) return NULL;

    char *pos = strstr(url, key);
    if (pos) {
        pos += strlen(key);  // key 다음부터 검색어 시작
        char *end = strpbrk(pos, "& ");
        int len = end ? (end - pos) : strlen(pos);

        char *raw = (char*)malloc(len + 1);
        strncpy(raw, pos, len);
        raw[len] = '\0';

        char *decoded = (char*)malloc(len * 3); // URL decode 후에도 충분히 크게
        url_decode(raw, decoded);
        free(raw);

        return decoded;
    }
    return NULL;
}

// 문자열에서 부분 문자열을 찾아 인덱스를 반환하는 함수
// 찾으면 해당 위치(0부터 시작), 없으면 -1 반환
int find_substring_index(char *str, const char *substr) {
    if (!str || !substr) return -1;

    char *pos = strstr(str, substr);
    if (pos == NULL) {
        return -1; // 부분 문자열 없음
    }
    return (int)(pos - str); // 포인터 차이를 인덱스로 변환
}

// 문자열 복사 (strdup 대체용)
char* my_strdup(char *s) {
    char *dup = (char*)malloc(strlen(s) + 1);
    if (dup) strcpy(dup, s);
    return dup;
}

// 문자열 배열에 추가
void add_part(char ***parts, int *size, int *capacity, char *token) {
    if (*size >= *capacity) {
        *capacity *= 2;
        *parts = (char**)realloc(*parts, sizeof(char*) * (*capacity));
    }
    (*parts)[(*size)++] = my_strdup(token);
}

// URL 분리 함수 (/, ., ?, =, & 모두 구분자로 사용)
char** split_url_all(char *url, int *count) {
    if (!url) return NULL;

    char *url_copy = my_strdup(url);
    if (!url_copy) return NULL;

    int capacity = 20;
    int size = 0;
    char **parts = (char**)malloc(sizeof(char*) * capacity);

    // 1) 프로토콜 추출
    char *protocol_end = strstr(url_copy, "://");
    char *host_start = url_copy;
    if (protocol_end) {
        *protocol_end = '\0';
        add_part(&parts, &size, &capacity, url_copy); // protocol
        host_start = protocol_end + 3;
    }

    // 2) 호스트 전체 추출
    char *path_start = strpbrk(host_start, "/?&="); // host 끝 찾기
    if (path_start) *path_start = '\0';
    add_part(&parts, &size, &capacity, host_start); // full host

    // 3) 호스트 세부 분리 (. 기준)
    char *host_copy = my_strdup(host_start);
    char *token = strtok(host_copy, ".");
    while (token) {
        add_part(&parts, &size, &capacity, token);
        token = strtok(NULL, ".");
    }
    free(host_copy);

    // 4) 나머지 부분 분리 (/ ? = & 기준)
    if (path_start) {
        token = strtok(path_start + 1, "/?=&");
        while (token) {
            add_part(&parts, &size, &capacity, token);
            token = strtok(NULL, "/?=&");
        }
    }

    *count = size;
    free(url_copy);
    return parts;
}


int is_valid_utf8_char(const unsigned char *s, int len) {
    if (len < 1) return 0;

    if (s[0] <= 0x7F) return 1;  // 1-byte ASCII
    if ((s[0] & 0xE0) == 0xC0 && len >= 2 &&
        (s[1] & 0xC0) == 0x80) return 2;  // 2-byte
    if ((s[0] & 0xF0) == 0xE0 && len >= 3 &&
        (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) return 3;  // 3-byte
    if ((s[0] & 0xF8) == 0xF0 && len >= 4 &&
        (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) return 4;  // 4-byte

    return 0;  // invalid UTF-8
}

void filter_utf8(const char *input, char *output, size_t out_size) {
    size_t i = 0, o = 0;
    size_t len = strlen(input);

    while (i < len && o < out_size - 1) {
        int valid_len = is_valid_utf8_char((const unsigned char *)&input[i], len - i);
        if (valid_len > 0 && o + valid_len < out_size - 1) {
            // Copy valid UTF-8 sequence
            memcpy(&output[o], &input[i], valid_len);
            o += valid_len;
            i += valid_len;
        } else {
            // Skip invalid byte
            i++;
        }
    }

    output[o] = '\0';  // Null-terminate result
}


int GetTotalRecordNum(mongoc_collection_t* pmgcolThisCollection, long* plTotalRecordNum)
{
    bson_error_t error;
    bson_t *pThisFilter = bson_new();
    
//    count = mongoc_collection_count_documents (pmgcolThisCollection, NULL, NULL, NULL, NULL, &error);
//    *plTotalRecordNum = (long)mongoc_collection_count(pmgcolThisCollection, MONGOC_QUERY_NONE, NULL, 0, 0, NULL, &error);
    *plTotalRecordNum = (long)mongoc_collection_count_documents(pmgcolThisCollection, pThisFilter, NULL, NULL, NULL, &error);
    if (*plTotalRecordNum < 0) 
    {
        printf("%s\n", error.message);
        fprintf (stderr, "%s\n", error.message);
        return 401;
    }
    bson_destroy(pThisFilter);

    return 0;
}

int GetRecordNumByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, long* plTotalRecordNum)
{
    bson_t *pThisQuery;
    bson_error_t error;

//    count = mongoc_collection_count_documents (pmgcolThisCollection, NULL, NULL, NULL, NULL, &error);
    pThisQuery = BCON_NEW ("cid", BCON_UTF8(caCID));
    *plTotalRecordNum = (long)mongoc_collection_count_documents(pmgcolThisCollection, pThisQuery, NULL, NULL, NULL, &error);
    bson_destroy (pThisQuery);
    if (*plTotalRecordNum < 0) 
    {
        printf("%s\n", error.message);
        fprintf (stderr, "%s\n", error.message);
        return 401;
    }
    
    return 0;
}

// 도큐먼트에서 _id 추출 후 문자열로 변환하는 함수
void GetObjectIdFromDocument(const bson_t* doc, char* out_oid_str, size_t buf_size)
{
    bson_iter_t iter;

    if (bson_iter_init_find(&iter, doc, "_id") && BSON_ITER_HOLDS_OID(&iter)) {
        const bson_oid_t* oid = bson_iter_oid(&iter);
        bson_oid_to_string(oid, out_oid_str);
    }
    else {
        snprintf(out_oid_str, buf_size, "");
    }
}

void print_string_array(const bson_t* doc, const char* field_name, char val[][NAME_SIZE], int* count)
{
    *count = 0;

    bson_iter_t iter;
    if (bson_iter_init_find(&iter, doc, field_name) && BSON_ITER_HOLDS_ARRAY(&iter)) {
        const uint8_t* array_buf;
        uint32_t array_len;
        bson_t array;

        bson_iter_array(&iter, &array_len, &array_buf);
        bson_init_static(&array, array_buf, array_len);

        bson_iter_t arr_iter;
        if (bson_iter_init(&arr_iter, &array)) {
#ifdef MANUAL_DEBUG
            printf("Field \"%s\" values:\n", field_name);
#endif
            while (bson_iter_next(&arr_iter)) {
                if (BSON_ITER_HOLDS_UTF8(&arr_iter)) {
                    const char* str = bson_iter_utf8(&arr_iter, NULL);
                    strcpy(val[*count], str);
                    val[*count][NAME_SIZE - 1] = '\0';  // 안전한 문자열 종료
#ifdef MANUAL_DEBUG
                    printf("  [%d] %s\n", *count, val[*count]);
#endif
                    (*count)++;
                }
            }
#ifdef MANUAL_DEBUG
            printf("  Array size: %d\n\n", *count);
#endif
        }
    }
    else {
        printf("Field \"%s\" not found or not an array.\n\n", field_name);
    }
}

// 특정 필드 이름을 받아 배열 크기와 값을 출력하는 함수
void print_double_array(const bson_t* doc, const char* field_name, double* val, int* count) 
{
    *count = 0;

    bson_iter_t iter;
    if (bson_iter_init_find(&iter, doc, field_name) && BSON_ITER_HOLDS_ARRAY(&iter)) {
        const uint8_t* array_buf;
        uint32_t array_len;
        bson_t array;

        bson_iter_array(&iter, &array_len, &array_buf);
        bson_init_static(&array, array_buf, array_len);

        bson_iter_t arr_iter;
        if (bson_iter_init(&arr_iter, &array)) {
#ifdef MANUAL_DEBUG
            printf("Field \"%s\" values:\n", field_name);
#endif
            while (bson_iter_next(&arr_iter)) {
                if (BSON_ITER_HOLDS_DOUBLE(&arr_iter)) {
                    val[*count] = bson_iter_double(&arr_iter);
#ifdef MANUAL_DEBUG
                    printf("  [%d] %.6f\n", count, val[*count]);
#endif
                    (*count)++;
                }
            }
#ifdef MANUAL_DEBUG
            printf("  Array size: %d\n\n", *count);
#endif
        }
    }
    else {
        printf("Field \"%s\" not found or not an array.\n\n", field_name);
    }
}

int GetAllPromotions(mongoc_collection_t* pmgcolThisCollection, long plTotalRecordNum, \
        PromotionDocu* pPromotion, int iChunkNum, long *lpSavedNum)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    long l, lSavedNum;


    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = bson_new();
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    

    printf("In GetAllPromotions() - Start : Reading promotions records\n");

    l = lSavedNum = 0 ;
    while((mongoc_cursor_next(pThisCursor, &pThisDoc)&&((l/CHUNK_SIZE) <= iChunkNum)))
    {
        if((l/CHUNK_SIZE) == iChunkNum)
        {
            pThisStr = bson_as_json(pThisDoc, NULL);
            if (pThisStr == NULL) {
                fprintf(stderr, "bson_as_json() failed — document skipped.\n");
                continue;  // 또는 break; 적절히 처리
            }

            ThisDocument.Parse(pThisStr);
            if (ThisDocument.HasParseError()) {
                fprintf(stderr, "JSON parse error at record %ld\n", l);
                bson_free(pThisStr);
                continue;
            }
            
            GetObjectIdFromDocument(pThisDoc, pPromotion[(l % CHUNK_SIZE)]._id , READ_BUF_SIZE - 1);

            strcpy(pPromotion[(l % CHUNK_SIZE)].PromotionID, ThisDocument["PromotionID"].GetString());
            
            bson_iter_t iter1;
            if (bson_iter_init_find(&iter1, pThisDoc, "RegisteredDate") &&
                BSON_ITER_HOLDS_DATE_TIME(&iter1)) {
                int64_t millis1 = bson_iter_date_time(&iter1);
                pPromotion[(l % CHUNK_SIZE)].RegisteredDate = millis1;   // UTC 밀리초
            }
            bson_iter_t iter2;
            if (bson_iter_init_find(&iter2, pThisDoc, "StartDate") &&
                BSON_ITER_HOLDS_DATE_TIME(&iter2)) {
                int64_t millis2 = bson_iter_date_time(&iter2);
                pPromotion[(l % CHUNK_SIZE)].StartDate = millis2;   // UTC 밀리초
            }
            bson_iter_t iter3;
            if (bson_iter_init_find(&iter3, pThisDoc, "EndDate") &&
                BSON_ITER_HOLDS_DATE_TIME(&iter3)) {
                int64_t millis3 = bson_iter_date_time(&iter3);
                pPromotion[(l % CHUNK_SIZE)].EndDate = millis3;   // UTC 밀리초
            }

            print_string_array(pThisDoc, "ArticleIDs", \
                    pPromotion[(l % CHUNK_SIZE)].ArticleIDs, &(pPromotion[(l % CHUNK_SIZE)].ArtIDsNum));
            print_string_array(pThisDoc, "QueryKeywords", \
                    pPromotion[(l % CHUNK_SIZE)].QueryKeywords, &(pPromotion[(l % CHUNK_SIZE)].QueryKeywordsNum));
            
            strcpy(pPromotion[(l % CHUNK_SIZE)].ClassAxis, ThisDocument["ClassAxis"].GetString());
            strcpy(pPromotion[(l % CHUNK_SIZE)].WriterAxis, ThisDocument["WriterAxis"].GetString());

            int iDummy;
            print_double_array(pThisDoc, "NormalClass", \
                    pPromotion[(l % CHUNK_SIZE)].NormalClass, &(pPromotion[(l % CHUNK_SIZE)].TotalClassNum));
            print_double_array(pThisDoc, "NormalWriter", \
                    pPromotion[(l % CHUNK_SIZE)].NormalWriter, &(pPromotion[(l % CHUNK_SIZE)].TotalWriterNum));
            print_double_array(pThisDoc, "ArticleEmbedding", pPromotion[(l % CHUNK_SIZE)].ArticleEmbedding, &iDummy);
            print_double_array(pThisDoc, "QueryEmbedding", pPromotion[(l % CHUNK_SIZE)].QueryEmbedding, &iDummy);
            
            lSavedNum++;
            printf("%ld / %ld\r", l+1, plTotalRecordNum);

            bson_free(pThisStr);
        }

        l++;
        if(l >= plTotalRecordNum) break;
    }
    *lpSavedNum = lSavedNum;
    printf("\n");
    printf("In GetAllPromotions() - End : Reading promotions records\n");

    mongoc_cursor_destroy (pThisCursor);    
    return 0;
}

int GetEventsByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, EventDatum* pEvent, \
        long plTotalRecordNum, int iChunkNum, long *lpSavedNum)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    long l, lSavedNum;


    pThisQuery = BCON_NEW("cid", BCON_UTF8(caCID));
    pThisOpts = bson_new();
    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    //pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
    //pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    printf("In GetEventsByCID() - Start : Reading events records\n");    
    l = lSavedNum = 0 ;
    while((mongoc_cursor_next(pThisCursor, &pThisDoc)&&((l/CHUNK_SIZE) <= iChunkNum)))
    {            
        if((l/CHUNK_SIZE) == iChunkNum)
        {
            strcpy(pEvent[(l % CHUNK_SIZE)].cid, "");
            strcpy(pEvent[(l % CHUNK_SIZE)].created_at, "");
            strcpy(pEvent[(l % CHUNK_SIZE)].properties_referrer, "");
            strcpy(pEvent[(l % CHUNK_SIZE)].properties_value, "");
            pEvent[(l % CHUNK_SIZE)].EventTime = (time_t)0;
            strcpy(pEvent[(l % CHUNK_SIZE)].ART_ID, "");
            strcpy(pEvent[(l % CHUNK_SIZE)].Query, "");

            pThisStr = bson_as_json(pThisDoc, NULL);
            if (pThisStr == NULL) {
                fprintf(stderr, "bson_as_json() failed — document skipped.\n");
                continue;  // 또는 break; 적절히 처리
            }

            ThisDocument.Parse(pThisStr);
            if (ThisDocument.HasParseError()) {
                fprintf(stderr, "JSON parse error at record %ld\n", l);
                bson_free(pThisStr);
                continue;
            }

            strcpy(pEvent[(l % CHUNK_SIZE)].cid, ThisDocument["cid"].GetString());
            strcpy(pEvent[(l % CHUNK_SIZE)].created_at, ThisDocument["created_at"].GetString());
            strcpy(pEvent[(l % CHUNK_SIZE)].properties_value, ThisDocument["properties_value"].GetString());
            strcpy(pEvent[(l % CHUNK_SIZE)].properties_referrer, ThisDocument["properties_referrer"].GetString());

            bson_free(pThisStr);
            
            pEvent[(l % CHUNK_SIZE)].EventTime = ISO8601StrToLocalTime(pEvent[(l % CHUNK_SIZE)].created_at);
            
            const char *keys[] = {"id_art="};
            char *value = NULL;
            for (int k = 0; k < 1; k++) {
                value = extract_value(pEvent[(l % CHUNK_SIZE)].properties_value, keys[k]);
                if (value) break; // 찾으면 중단
            }
            if (value)
            {
                keep_digits(value);
                strcpy(pEvent[(l % CHUNK_SIZE)].ART_ID, value);
                free(value);
            }
            else
            {
                int count = 0;
                char **parts = split_url_all(pEvent[(l % CHUNK_SIZE)].properties_value, &count);
                for (int i = 0; i < count; i++) {
                    if(!strcmp("read", parts[i]))
                    {
                        keep_digits(parts[i+1]);
                        strcpy(pEvent[(l % CHUNK_SIZE)].ART_ID, parts[i+1]);
                    }
                    free(parts[i]); // 해제
                }
                free(parts);
            }

            const char *keys2[] = {"q=", "query=", "search="};
            char *value2 = NULL;
            bool exist = false;
            for (int k = 0; k < 3; k++) {
                value2 = extract_value(pEvent[(l % CHUNK_SIZE)].properties_referrer, keys2[k]);
                if (value2) break; // 찾으면 중단
            }
            if (value2)
            {
                strcpy(pEvent[(l % CHUNK_SIZE)].Query, value2);
                exist = true;
                free(value2);
            }
            for (int k = 0; k < 3; k++) {
                value2 = extract_value(pEvent[(l % CHUNK_SIZE)].properties_value, keys2[k]);
                if (value2) break; // 찾으면 중단
            }
            if (value2)
            {
                if(exist)
                {
                    strcat(pEvent[(l % CHUNK_SIZE)].Query, ", ");
                    strcat(pEvent[(l % CHUNK_SIZE)].Query, value2);
                }
                else
                {
                    strcpy(pEvent[(l % CHUNK_SIZE)].Query, value2);                    
                }
                free(value2);
            }
            
            lSavedNum++;
            printf("%ld / %ld\r", l+1, plTotalRecordNum);
        }

        l++;
        if(l >= plTotalRecordNum) break;
    }
    *lpSavedNum = lSavedNum;
    printf("\n");
    printf("In GetEventsByCID() - End : Reading events records\n");

    mongoc_cursor_destroy (pThisCursor);    
    return 0;
}

int GetTotalEventSizeAndAxesByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, \
        long* lTotalEventSize, char* caClassAxis, char* caWriterAxis)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr=NULL;
    Document ThisDocument;


    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = BCON_NEW("cid", BCON_UTF8(caCID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    if(mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            *lTotalEventSize = 0;
            caClassAxis[0] = caWriterAxis[0] = '\0';
            return 400;
        }

        auto iTES = ThisDocument.FindMember("TotalEventSize");
        if((iTES != ThisDocument.MemberEnd())&&(iTES->value.IsInt64()))
        {
            *lTotalEventSize = iTES->value.GetInt64();            
        }
        else
        {
            *lTotalEventSize = 0;            
        }
        
        auto iCA = ThisDocument.FindMember("ClassAxis");
        if((iCA != ThisDocument.MemberEnd())&&(iCA->value.IsString()))
        {
            strcpy(caClassAxis, iCA->value.GetString());
        }
        else
        {
            strcpy(caClassAxis, "");
        }
        
        auto iWA = ThisDocument.FindMember("WriterAxis");
        if((iWA != ThisDocument.MemberEnd())&&(iWA->value.IsString()))
        {
            strcpy(caWriterAxis, iWA->value.GetString());       
        }
        else
        {
            strcpy(caWriterAxis, "");               
        }
        
        bson_free(pThisStr);
    }
    else
    {
        *lTotalEventSize = 0;
        strcpy(caClassAxis, "");
        strcpy(caWriterAxis, "");               
    }

    mongoc_cursor_destroy (pThisCursor);    
    return 0;
}

int ConToMemAsJson(char* caCMD, char* caRSLT)
{
    FILE *fp;
    char buffer[STR_SIZE];
    
    fp = popen(caCMD, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 1;
    }

    caRSLT[0] = '\0';
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(caRSLT, buffer);
    }
    caRSLT[1] = '\"';
    caRSLT[11] = '\"';

    pclose(fp);

#ifdef MANUAL_DEBUG
    printf("\nIn ConToMemAsJson\n");
    printf("%s\n", caRSLT);
#endif
    
    return 0;
}

int GetEmbFromText(char* caText, double* dTextEmb, bool bLarge)
{
    if(!strcmp(caText, ""))
    {
        for(int i=0; i<EMB_SIZE; i++)
        {
            dTextEmb[i] = 0;
        }
        return 123;
    }

    char caTmpText[TEXT_SIZE*20];
    filter_utf8(caText, caTmpText, sizeof(caTmpText));
    sanitize_shell_input(caTmpText);
    strcpy(caText, caTmpText);

    printf("\nIn GetEmbFromText() : caText is %s\n",caText);
    
    char caCMD[TEXT_SIZE*20]={0};
    if(bLarge)
    {
        strcpy(caCMD, "python3  docu_embedding_vertex.py  \'");
        strncat(caCMD, caText, sizeof(caCMD) - strlen(caCMD) -1);
        strcat(caCMD, "\'");
    }
    else
    {
        strcpy(caCMD, "python3  query_embedding_vertex.py  \"");
        strncat(caCMD, caText, sizeof(caCMD) - strlen(caCMD) -1);
        strcat(caCMD, "\"");       
    }
    ConToMemAsJson(caCMD, caResult);
#ifdef MANUAL_DEBUG
    printf("\nIn GetEmbFromText() : caResult is %s\n",caResult);
#endif
    
    Document ThisDocument;
    ThisDocument.Parse(caResult);
    if (!ThisDocument.IsObject()) {
        fprintf(stderr, "Invalid JSON response in GetEmbFromText(): not an object\n");
        exit(1);
    }
    if (ThisDocument.HasMember("embedding") && ThisDocument["embedding"].IsArray()) {
        const Value& arr = ThisDocument["embedding"];
        for (SizeType i = 0; i < arr.Size(); i++) {
            dTextEmb[i] = arr[i].GetDouble();  // ← double 값 읽기
        }
    }
#ifdef MANUAL_DEBUG
    printf("In GetEmbFromText() : dTextEmb\n");
    for(int j=0; j<EMB_SIZE; j++)
    {
        printf("%lf, ", dTextEmb[j]);
    }
    printf("\n");
#endif

    return 0;
}

int GetTotalRecNum(MYSQL* DB_connection, MYSQL* DB_conn, \
        char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, \
        long *ipTotalNum)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row; 
    char caSQL[STR_SIZE];
    
    strcpy(caSQL, "SELECT COUNT(*) FROM ");
    strcat(caSQL, TB_NAME);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                *ipTotalNum = atoi(DB_row[0]);                
            }
            else
            {
                *ipTotalNum = 0;
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    
    return 0;
}

int GetAxis(MYSQL* DB_connection, MYSQL* DB_conn, char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, \
        char* COL_NAME, char* TB_NAME, \
        int iTotalNum, char** caaAxis)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row; 
    char caSQL[STR_SIZE];

    strcpy(caSQL, "SELECT ");
    strcat(caSQL, COL_NAME);
    strcat(caSQL, " FROM ");
    strcat(caSQL, TB_NAME);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        int i=0;
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                strcpy(caaAxis[i], DB_row[0]);                
            }
            else
            {
                caaAxis[i][0] = '\0';
            }
            i++;
            if(i>=iTotalNum) break;
        }
        mysql_free_result(DB_result);
#ifdef MANUAL_DEBUG
        printf("\nTotal classes number is %d again\n", i);
#endif
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    
    return 0;
}

int GetClassIdxWriterIdxEmbFromArtID(MYSQL* DB_connection, MYSQL* DB_conn, \
        char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, \
        char** caaClassAxis, char** caaWriterAxis, \
        int iTotalClassNum, int iTotalWriterNum, char* ART_ID, \
        int* iClassIdx, int* iWriterIdx, double* dTmpArticleEmb)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row; 
    char caSQL[STR_SIZE], caTmp[READ_BUF_SIZE]={0};
    
    printf("\nIn GetClassIdxWriterIdxEmbFromArtID() : ART_ID is %s\n", ART_ID);
    
    strcpy(caSQL, "SELECT CODE_ID FROM naeil_wms_db.WMS_ARTICLE_CODE WHERE ART_ID = ");
    strcat(caSQL, ART_ID);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                strcpy(caTmp, DB_row[0]);                
            }
            else
            {
                caTmp[0] = '\0';
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    caSQL[0] = '\0';
    
    printf("In GetClassIdxWriterIdxEmbFromArtID() : CLASS_ID is %s\n", caTmp);
    
    bool bClassFault=true;
    for(int i=0; i<iTotalClassNum; i++)
    {
        if(!strcmp(caaClassAxis[i], caTmp))
        {
            *iClassIdx = i;
            bClassFault = false;
            break;
        }
    }
    if(bClassFault) *iClassIdx = -1;
    caTmp[0] = '\0';
    
    printf("In GetClassIdxWriterIdxEmbFromArtID() : iClassIdx is %d\n", *iClassIdx);
    
    strcpy(caSQL, "SELECT USER_ID FROM naeil_wms_db.WMS_ARTICLE_WRITER WHERE ART_ID = ");
    strcat(caSQL, ART_ID);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                strcpy(caTmp, DB_row[0]);                
            }
            else
            {
                caTmp[0] = '\0';
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    caSQL[0] = '\0';

    printf("In GetClassIdxWriterIdxEmbFromArtID() : WRITER_ID is %s\n", caTmp);

    bool bWriterFault=true;
    for(int i=0; i<iTotalWriterNum; i++)
    {
        if(!strcmp(caaWriterAxis[i], caTmp))
        {
            *iWriterIdx = i;
            bWriterFault = false;
            break;
        }
    }
    if(bWriterFault) *iWriterIdx = -1;
    caTmp[0] = '\0';
    
    printf("In GetClassIdxWriterIdxEmbFromArtID() : iWriterIdx is %d\n", *iWriterIdx);

    char caText[TEXT_SIZE*20];
    strcpy(caSQL, "SELECT TITLE FROM naeil_wms_db.WMS_ARTICLE WHERE ART_ID = ");
    strcat(caSQL, ART_ID);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                strcpy(caText, DB_row[0]);                
            }
            else
            {
                caText[0] = '\0';
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    caSQL[0] = '\0';

    strcat(caText, "\n");
    strcpy(caSQL, "SELECT BODY_TEXT FROM naeil_wms_db.WMS_ARTICLE_BODY WHERE ART_ID = ");
    strcat(caSQL, ART_ID);
    if(mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while((DB_row = mysql_fetch_row(DB_result)) != NULL){
            if(DB_row[0])
            {
                strncat(caText, DB_row[0], sizeof(caText) - strlen(caText) - 1);
            }
            else
            {
                caText[0] = '\0';
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if(iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
        }
    }
    caSQL[0] = '\0';
    
    GetEmbFromText(caText, dTmpArticleEmb, true);

    return 0;    
}


int Normalize(double* dNormal, int iTotalNum)
{
    double dMax=dNormal[0];
    
    for(int i=0; i<iTotalNum; i++)
    {
        if(dMax < dNormal[i]) dMax = dNormal[i];
    }
    if(dMax!=0)
    {
         for(int i=0; i<iTotalNum; i++)
        {
            dNormal[i] = dNormal[i]/dMax;
        }       
    }
    
    return 0;
}

/* 인덱스 clamp: [0, n-1] 범위로 고정 */
static inline int clamp(int i, int n) {
    if (i < 0)     return 0;
    if (i >= n)    return n - 1;
    return i;
}

/* 길이 32의 가우시안 커널 생성 (정규화 포함)
 * sigma: 표준편차 (예: 4.0 ~ 6.0 권장; 대략 KLEN/6 ≈ 5.3 근처가 무난)
 * anchor: 커널 기준 인덱스(정수 중심). 짝수 길이에서는 KLEN/2 사용.
 */
static void build_gaussian_kernel(double kernel[KLEN], double sigma) {
    const int anchor = KLEN / 2;          /* 짝수 길이 32 → anchor = 16 */
    double sum = 0.0;

    for (int i = 0; i < KLEN; ++i) {
        double x = (double)(i - anchor);  /* 정수 중심 기준 오프셋 */
        double w = exp(-(x * x) / (2.0 * sigma * sigma));
        kernel[i] = w;
        sum += w;
    }
    /* 정규화 */
    for (int i = 0; i < KLEN; ++i) {
        kernel[i] /= sum;
    }
}

/* 1D Gaussian Smoothing
 * in  : 입력 신호 (float)
 * out : 출력 신호 (float)
 * n   : 신호 길이
 * sigma: 가우시안 표준편차 = 5.33; /* 길이 32 커널에서 무난한 값 (대략 KLEN/6)
 */
void gaussian_smooth_1d(const double *in, double *out, int n, double sigma) {
    double k[KLEN];
    const int anchor = KLEN / 2;
    build_gaussian_kernel(k, sigma);

    for (int i = 0; i < n; ++i) {
        double acc = 0.0;
        for (int m = 0; m < KLEN; ++m) {
            int idx = clamp(i + (m - anchor), n);
            acc += (double)in[idx] * k[m];
        }
        out[i] = (double)acc;
    }
}

//  배열 data[]의 시작 인덱스가 0이 아닌 1이라는 사실에 유의
//  따라서 data[]의 크기는 (nn*2 + 1)이다.
void four1(double data[], unsigned long nn, int isign)
{
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	double tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=nn;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=isign*(6.28318530717959/mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}

//  배열 data[]의 시작 인덱스가 0이 아닌 1이라는 사실에 유의
//  따라서 data[]의 크기는 (nn*2 + 1)이다.
//  반면에 dRslt[]의 크기는 nn이며 시작 인덱스는 0이다.
int PowerSpectralDensity(double data[], int nn, double* dRslt)
{
    for(int i=0; i<nn; i++)
    {
        dRslt[i] = sqrt(pow(data[i*2 +1], 2) + pow(data[i*2 +2], 2));
    }
    
    return 0;
}

int DWT(double* dFilter, int iFilterKlen, int iLevel, double* dInput, int iInputLen, double* dOutput)
{
    if(iLevel <= 0) iLevel = 1;
    
    int iFiltLen = (iFilterKlen-1)*pow(2,iLevel-1)+1;
    double* dFilt = (double*)malloc(sizeof(double)*iFiltLen);
    for(int i=0; i<iFilterKlen; i++)
    {
        for(int j=(i*pow(2,iLevel-1)); j<((i*pow(2,iLevel-1))+pow(2,iLevel-1)); j++)
        {
            if(j==(i*pow(2,iLevel-1)))
            {
                dFilt[j] = dFilter[i];
                if(i == (iFilterKlen-1)) break;
            }
            else
            {
                dFilt[j] = 0;
            }
        }
    }

    const int anchor = (int)round((double)iFiltLen / (double)2);
    for (int i = 0; i < iInputLen; ++i)
    {
        double acc = 0.0;
        for (int k = 0; k < iFiltLen; ++k)
        {
            int idx = clamp((i - k + anchor), iInputLen);
            acc += (double)dInput[idx] * dFilt[k];
        }
        dOutput[i] = (double)acc;
    }

    free(dFilt);
    
    return 0;
}

int DWTBank(double* dLPFilter, int iLPFilterKlen, double* dHPFilter, int iHPFilterKlen, \
        int iLevel, double* dInput, int iInputLen, double** dOutput)
{
    /* Original DWT Version
    double *dLowPassed = (double*)malloc(sizeof(double)*iInputLen);
    double *dLowPassedTmp = (double*)malloc(sizeof(double)*iInputLen);
    for(int i=0; i<iInputLen; i++)
    {
        dLowPassedTmp[i] = dInput[i];
    }
    
    for(int i=0; i<iLevel; i++)
    {   
        DWT(dHPFilter, iHPFilterKlen, i+1, dLowPassedTmp, iInputLen, dOutput[i]);
        DWT(dLPFilter, iLPFilterKlen, i+1, dLowPassedTmp, iInputLen, dLowPassed);
        for(int j=0; j<iInputLen; j++)
        {
            dLowPassedTmp[j] = dLowPassed[j];
        }
    }
    
    free(dLowPassed);
    free(dLowPassedTmp);
    
    return 0;
    */

    for(int i=0; i<iInputLen; i++)
    {
        dOutput[0][i] = dInput[i];
    }
    
    for(int i=0; i<iLevel-1; i++)
    {   
        DWT(dLPFilter, iLPFilterKlen, i+1, dOutput[i], iInputLen, dOutput[i+1]);
    }
    
    return 0;
}

int GetDyadicLevel(int iInput, int* iLevel)
{
    int iTmp, iQ, iR;
    iTmp = iInput;
    *iLevel=0;
    do
    {
        iQ = iTmp/2;
        iR = iTmp%2;
        iTmp = iQ;
        
        if(iR!=0) break;
        (*iLevel)++;
    } while(true);
    
    return 0;
}

int GetAcceleration(double** dInput, int iInputColLen, int iLevel, double* dOutput)
{
    int *iIdx = (int*)malloc(sizeof(int)*iInputColLen);
    
    for(int i=0; i<iInputColLen; i++)
    {
        double dMax = dInput[0][i];
        iIdx[i] = 0;
        for(int j=0; j<iLevel; j++)
        {
            if(dInput[j][i] > dMax)
            {
                dMax = dInput[j][i];
                iIdx[i] = j;
            }
#ifdef MANUAL_DEBUG
                printf("\nIn GetAcceleration() : dInput[%d][%d] = %lf, ", i, j, dInput[i][j]);
#endif
        }
    }
    for(int i=0; i<iInputColLen; i++)
    {
        dOutput[i] = pow(2, iIdx[i]);
    }
    
    free(iIdx);
    
    return 0;
}

static bool append_double_array(bson_t *parent, const char *field_name,
                                const double *vals, size_t n) 
{
    bson_t arr;
    char key_buf[16];
    const char *keyptr;            // ← 여기!
    if (!bson_append_array_begin(parent, field_name, -1, &arr)) {
        return false;
    }

    for (uint32_t i = 0; i < (uint32_t)n; i++) {
        // 배열 인덱스는 문자열 키("0","1",...)로 넣어야 합니다.
        // 두 번째 인자에 &keyptr (NULL 금지), 세 번째는 백업 버퍼
        bson_uint32_to_string(i, &keyptr, key_buf, sizeof key_buf);
        if (!bson_append_double(&arr, keyptr, -1, vals[i])) {
            bson_append_array_end(parent, &arr);
            return false;
        }
    }

    if (!bson_append_array_end(parent, &arr)) {
        return false;
    }
    return true;
}

int InsertDocuInAnalsCollection(mongoc_collection_t* pmgcolThisCollection, \
        char* cid, long TotalEventSize, char* ClassAxis, char* WriterAxis, \
        double* Period, int iPeriodSize, double* Acceleration, int iAccelerationSize, \
        double* NormalClass, int iNormalClassSize, double* NormalWriter, int iNormalWriterSize, \
        double* ArticleAccumEmbedding, int iArticleAccumEmbeddingSize, double* QueryAccumEmbedding, int iQueryAccumEmbeddingSize)
{
    bson_t *pThisInsert;
    bson_error_t ThisError;
    bool bOK=false;

    pThisInsert = bson_new();
    if (!pThisInsert) {
        fprintf(stderr, "bson_new failed in InsertDocuInAnalsCollection()\n");
        return 301;
    }

    if (!BSON_APPEND_UTF8(pThisInsert, "cid", cid)) {
        fprintf(stderr, "append cid failed in InsertDocuInAnalsCollection()\n");
        return 302;
    }
    if (!BSON_APPEND_INT64(pThisInsert, "TotalEventSize", TotalEventSize)) {
        fprintf(stderr, "append TotalEventSize failed in InsertDocuInAnalsCollection()\n");
        return 302;
    }
    if (!BSON_APPEND_UTF8(pThisInsert, "ClassAxis", ClassAxis)) {
        fprintf(stderr, "append ClassAxis failed in InsertDocuInAnalsCollection()\n");
        return 303;
    }
    if (!BSON_APPEND_UTF8(pThisInsert, "WriterAxis", WriterAxis)) {
        fprintf(stderr, "append WriterAxis failed in InsertDocuInAnalsCollection()\n");
        return 304;
    }

    if (!append_double_array(pThisInsert, "Period", Period, iPeriodSize)) {
        fprintf(stderr, "append array Period failed in InsertDocuInAnalsCollection()\n");
        return 305;
    }
    if (!append_double_array(pThisInsert, "Acceleration", Acceleration, iAccelerationSize)) {
        fprintf(stderr, "append array Acceleration failed in InsertDocuInAnalsCollection()\n");
        return 306;
    }
    if (!append_double_array(pThisInsert, "NormalClass", NormalClass, iNormalClassSize)) {
        fprintf(stderr, "append array NormalClass failed in InsertDocuInAnalsCollection()\n");
        return 307;
    }
    if (!append_double_array(pThisInsert, "NormalWriter", NormalWriter, iNormalWriterSize)) {
        fprintf(stderr, "append array NormalWriter failed in InsertDocuInAnalsCollection()\n");
        return 308;
    }
    if (!append_double_array(pThisInsert, "ArticleAccumEmbedding", ArticleAccumEmbedding, iArticleAccumEmbeddingSize)) {
        fprintf(stderr, "append array ArticleAccumEmbedding failed in InsertDocuInAnalsCollection()\n");
        return 309;
    }
    if (!append_double_array(pThisInsert, "QueryAccumEmbedding", QueryAccumEmbedding, iQueryAccumEmbeddingSize)) {
        fprintf(stderr, "append array QueryAccumEmbedding failed in InsertDocuInAnalsCollection()\n");
        return 310;
    }

    bOK = mongoc_collection_insert_one(pmgcolThisCollection, pThisInsert, NULL, NULL, &ThisError);
    if (!bOK) {
        fprintf(stderr, "insert_one failed in InsertDocuInAnalsCollection(): %s\n", ThisError.message);
        return 311;
    }
    
    printf("\nSuccess : %s document for Anals_Collection is inserted\n", cid);
    return 0;
}

int GetDatesByPromoID(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, char* caToken, char* caID, \
    char* caRegDate, int iRegDSize, char* caStartDate, int iStartDSize, char* caEndDate, int iEndDSize)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char caSQL[STR_SIZE];

    strcpy(caSQL, "SELECT REG_DT, PERIOD_START_DT, PERIOD_END_DT FROM ");
    strcat(caSQL, TB_NAME);
    strcat(caSQL, " WHERE ");
    strcat(caSQL, caToken);
    strcat(caSQL, " = \'");
    strcat(caSQL, caID);
    strcat(caSQL, "\'");
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
            {
                strcpy(caRegDate, DB_row[0]);
            }
            else
            {
                caRegDate[0] = '\0';
            }
            if (DB_row[1])
            {
                strcpy(caStartDate, DB_row[1]);
            }
            else
            {
                caStartDate[0] = '\0';
            }
            if (DB_row[2])
            {
                strcpy(caEndDate, DB_row[2]);
            }
            else
            {
                caEndDate[0] = '\0';
            }
            break;
        }
        mysql_free_result(DB_result);
    }
    else
    {
        // 에러
        char caError[READ_BUF_SIZE];
        int iErrno = mysql_errno(DB_conn);
        strcpy(caError, mysql_error(DB_conn));
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }

    return 0;
}

bool IsAllZero(double *daInput, int iSize)
{
    for(int i = 0; i < iSize; i++)
    {
        if(daInput[i] != 0) return false;
    }
    return true;
}

int UpdateClassWriterInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    char *caClassAxis, char *caWriterAxis, double *daNormalClass, double *daNormalWriter, \
        int iNormalClassSize, int iNormalWriterSize)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateClassWriterInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // 문자열 필드: caClassAxis, caWriterAxis
            BSON_APPEND_UTF8(&set, "ClassAxis", caClassAxis);
            BSON_APPEND_UTF8(&set, "WriterAxis", caWriterAxis);
            
            // double array 필드
            append_double_array(&set, "NormalClass", daNormalClass, iNormalClassSize);
            append_double_array(&set, "NormalWriter", daNormalWriter, iNormalWriterSize);

            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateClassWriterInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateClassWriterInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

int UpdateRegDateInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    int64_t iRegisteredDate)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateRegDateInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // ISODate 필드
            BSON_APPEND_DATE_TIME(&set, "RegisteredDate", iRegisteredDate);
            
            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateRegDateInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateRegDateInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

int UpdateStartDateInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    int64_t iStartDate)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateStartDateInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // ISODate 필드
            BSON_APPEND_DATE_TIME(&set, "StartDate", iStartDate);
            
            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateStartDateInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateStartDateInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

int UpdateEndDateInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    int64_t iEndDate)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateEndDateInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // ISODate 필드
            BSON_APPEND_DATE_TIME(&set, "EndDate", iEndDate);
            
            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateEndDateInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateEndDateInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

int UpdateArticleEmbInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    double *daArtEmb, int iArtEmbSize)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateArticleEmbInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // double array 필드
            append_double_array(&set, "ArticleEmbedding", daArtEmb, iArtEmbSize);

            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateArticleEmbInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateArticleEmbInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

int UpdateQueryEmbInDocuByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, char *ca_id, \
    double *daQueryEmb, int iQueryEmbSize)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caTmp[READ_BUF_SIZE] = {0};


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", caPromoID);

    // (6) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (7) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (8) 결과 확인 및 업데이트
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            printf("UpdateQueryEmbInDocuByPromoID() : JSON이 아니거나 루트가 Object가 아님\n");
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return 201;
        }

        GetObjectIdFromDocument(pThisDoc, caTmp, READ_BUF_SIZE);

        if (strcmp(caTmp, ca_id) == 0)
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caTmp, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caTmp);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // double array 필드
            append_double_array(&set, "QueryEmbedding", daQueryEmb, iQueryEmbSize);

            bson_append_document_end(pUpdateDoc, &set);

            // (3) MongoDB 업데이트 실행
            bson_error_t error;
            if (!mongoc_collection_update_one(
                pmgcolThisCollection,               // 컬렉션 핸들
                pUpdateFilter,                 // 필터 (_id)
                pUpdateDoc,                    // 업데이트 문서
                NULL,                          // 옵션 없음
                NULL,                          // 결과 정보 불필요
                &error))                       // 에러 객체
            {
                printf("UpdateQueryEmbInDocuByPromoID() : 문서(%s, %s) 업데이트 실패\n", caPromoID, ca_id);
            }
            else
            {
                printf("UpdateQueryEmbInDocuByPromoID() : 문서(%s, %s) 업데이트 성공\n", caPromoID, ca_id);
            }

            // (4) 메모리 해제
            bson_destroy(pUpdateFilter);
            bson_destroy(pUpdateDoc);
        }

        bson_free(pThisStr);
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}



int main(int argc, char** argv) {
    long lSavedPromotionRecNum = 0;

    char caBigDataServerIP[READ_BUF_SIZE];
    caBigDataServerIP[0] = '\0';
    char caDBServerIP[READ_BUF_SIZE];
    caDBServerIP[0] = '\0';
    char caSavedPromotionRecNum[READ_BUF_SIZE];
    caSavedPromotionRecNum[0] = '\0';

    char BigData_uri_string[STR_SIZE];  // "mongodb://localhost:37017"
    BigData_uri_string[0] = '\0';
    mongoc_uri_t *BigData_uri;
    mongoc_client_t *BigData_client;
    mongoc_database_t *BigData_database;
    mongoc_collection_t *BigData_Promotions_collection;
    bson_error_t BigData_error = {0};

//    mongoc_cursor_t *pEventCursor, *pPromotionCursor, *pAnalCursor;
//    bson_t *pEventQuery, *pEventOpts, *pPromotionQuery, *pPromotionOpts, *pAnalQuery, *pAnalOpts;
//    const bson_t *pEventDoc, *pPromotionDoc, *pAnalDoc;
//    char *pEventStr, *pPromotionStr, *pAnalStr;
//    Document EventDocument, PromotionDocument, AnalDocument;

    MYSQL* DB_conn, DB_connection = {0};
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row = {0}; 
    char DB_HOST[NAME_SIZE];    //호스트명
    DB_HOST[0] = '\0';
    char DB_USER[NAME_SIZE];    //사용자명
    DB_USER[0] = '\0';
    char DB_PASS[NAME_SIZE];    //비밀번호
    DB_PASS[0] = '\0';
    char DB_NAME[NAME_SIZE];    //DB명
    DB_NAME[0] = '\0';
    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 시작
    ////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MANUAL_DEBUG
    printf("This is starting point.\n");
#endif
    FILE *fptr = fopen("PromotionAnalyzerSetting.set", "rt");
    if(fptr==NULL)
    {
        printf("\nPromotionAnalyzerSetting.set File should be exist.\n");
        return 201;
    }

    int rdcnt = 0;
    rdcnt = fscanf(fptr, "%s", caBigDataServerIP);            
    if(!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong BigDataServerIP in PromotionAnalyzerSetting.set\n");
        return 202;
    }

    rdcnt = fscanf(fptr, "%s", caDBServerIP);            
    if(!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong DBServerIP in PromotionAnalyzerSetting.set\n");
        return 203;
    }
    
    rdcnt = fscanf(fptr, "%s", caSavedPromotionRecNum);
    if(!rdcnt)
    {
        fclose(fptr);
        printf("Wrong visitor order number in PromotionAnalyzerSetting.set\n");
        return 212;
    }
    lSavedPromotionRecNum = atol(caSavedPromotionRecNum);

    fclose(fptr);
    printf("\nNaeil-CRM Big Data Server(for www.naeil.com) : IP = %s\n", caBigDataServerIP);
    printf("\nnaeil-msp-db Server(for www.naeil.com) : IP = %s\n", caDBServerIP);


    strcpy(BigData_uri_string, "");
    strcat(BigData_uri_string, caBigDataServerIP);
    strcat(BigData_uri_string, "");
    strcat(BigData_uri_string, "");

//  11.10.2.33번 서버 : naeil-msp-db
    strcpy(DB_HOST, caDBServerIP);    //호스트명
    strcpy(DB_USER, "");    //사용자명
    strcpy(DB_PASS, "");    //비밀번호
    strcpy(DB_NAME, "");    //DB명

    /*
    * Required to initialize libmongoc's internals
    */
    mongoc_init ();
    /*
    * Safely create a MongoDB URI object from the given string
    */
    BigData_uri = mongoc_uri_new_with_error (BigData_uri_string, &BigData_error);
    if (!BigData_uri) {
        printf ("failed to parse URI: %s\n"
               "error message:       %s\n",
               BigData_uri_string,
               BigData_error.message);
        fprintf (stderr,
               "failed to parse URI: %s\n"
               "error message:       %s\n",
               BigData_uri_string,
               BigData_error.message);
        return EXIT_FAILURE;
    }
    printf("\nSuccess : BigDataSvr is connected\n");
    /*
    * Create a new client instance
    */
    BigData_client = mongoc_client_new_from_uri (BigData_uri);
    if (!BigData_client) {
        printf("\nError : mongoc_client_new_from_uri\n");
        return EXIT_FAILURE;
    }
    printf("\nSuccess : BigDataSvr-Client is connected\n");
    /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples).
    */
    mongoc_client_set_appname (BigData_client, "PromotionAnalyzer");
    /*
    * Get a handle on the database "db_name" and collection "coll_name"
    */
    BigData_database = mongoc_client_get_database (BigData_client, "WwwNaeilCom2023");
    printf("\nSuccess : BigDataSvr-DB is connected\n");
    BigData_Promotions_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Promotions");
    printf("\nSuccess : BigDataSvr-PromotionsCollection is connected\n");

    mysql_init(&DB_connection);
    DB_conn = mysql_real_connect(&DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 13306, (char *)NULL, 0); 
    if (DB_conn == NULL) {
        // ❌ 실패: 에러 메시지 출력
        fprintf(stderr, "\nMySQL 연결 실패: %s\n", mysql_error(&DB_connection));
        return 1;
    } else {
        // ✅ 성공
        printf("\nMySQL 연결 성공!\n");
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
 
    
    
    bool bIsThisPromotionFirstTurn = true;
    long lPromotionAnalyzerLoopNum = 0;
    do
    {
        printf("\n  Start PromotionAnalyzer-Loop Number is %ld\n", lPromotionAnalyzerLoopNum);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 선언 : 시작
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        long iTotalClassNum = 0, iTotalWriterNum = 0;
        GetTotalRecNum(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, "naeil_wms_db.WMS_CODE_CLASS", \
                &iTotalClassNum);
        printf("\n  Total classes number is %d\n", iTotalClassNum);
        GetTotalRecNum(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, "naeil_wms_db.WMS_USER", \
                &iTotalWriterNum);
        printf("\n  Total writers number is %d\n", iTotalWriterNum);

        double dArticleEmb[EMB_SIZE] = {0}, dQueryEmb[EMB_SIZE] = {0};
        char **caaClassAxis, **caaWriterAxis;
        char caClassAxis[TEXT_SIZE] = {0}, caWriterAxis[TEXT_SIZE] = {0};
        double dNormalClass[iTotalClassNum] = {0}, dNormalWriter[iTotalWriterNum] = {0};
        
        caaClassAxis = (char**)malloc(sizeof(char*) * iTotalClassNum);
        for(int i=0; i<iTotalClassNum; i++)
        {
            caaClassAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
            caaClassAxis[i][0] = '\0';
        }
        caaWriterAxis = (char**)malloc(sizeof(char*) * iTotalWriterNum);
        for(int i=0; i<iTotalWriterNum; i++)
        {
            caaWriterAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
            caaWriterAxis[i][0] = '\0';
        }
        caClassAxis[0] = '\0'; caWriterAxis[0]='\0';
        
        GetAxis(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                "CODE_ID", "naeil_wms_db.WMS_CODE_CLASS", \
                iTotalClassNum, caaClassAxis);
        printf("    iClass: \n");
        for(int i=0; i<iTotalClassNum; i++)
        {
            int len = 0;
            len = strlen(caClassAxis);
            sprintf(caClassAxis + len, "%s, ", caaClassAxis[i]);
            printf("%s, ", caaClassAxis[i]);
        }
        printf("\n");

        GetAxis(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                "USER_ID", "naeil_wms_db.WMS_USER", \
                iTotalWriterNum, caaWriterAxis);
        printf("    iWriter: \n");
        for(int i=0; i<iTotalWriterNum; i++)
        {
            int len = 0;
            len = strlen(caWriterAxis);
            sprintf(caWriterAxis + len, "%s, ", caaWriterAxis[i]);
            printf("%s, ", caaWriterAxis[i]);
        }
        printf("\n");
        
        long lTotalPromotionNum = 0, lPromotionRecNum = 0;
        GetTotalRecordNum(BigData_Promotions_collection, &lTotalPromotionNum);
        printf("\n  Total promotions number is %ld.\n", lTotalPromotionNum);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 선언 : 끝
        ////////////////////////////////////////////////////////////////////////////////////////////////////

        if(bIsThisPromotionFirstTurn) //  프로그램이 시작될 때 PromotionAnalyzerSetting.set에서 읽어 온 값으로써 해당 Promotion에서 작업을 시작하라는 의미
        {
            lPromotionRecNum = (lSavedPromotionRecNum / CHUNK_SIZE) * CHUNK_SIZE;
            bIsThisPromotionFirstTurn = false;
        }
        else
        {
            lPromotionRecNum = 0;
        }
        PromotionDocu Promotions[CHUNK_SIZE] = {0};
        int iPrevChunkNum = 0, iCurrChunkNum = 0;
        iPrevChunkNum = -1;
        long lSavedNum = 0;
        while (lPromotionRecNum < lTotalPromotionNum) //  Promotion 순번대로 한 개씩 처리
        {
            iCurrChunkNum = lPromotionRecNum / CHUNK_SIZE;
            if(iPrevChunkNum != iCurrChunkNum)
            {
                GetAllPromotions(BigData_Promotions_collection, lTotalPromotionNum, Promotions, iCurrChunkNum, &lSavedNum);
                iPrevChunkNum = iCurrChunkNum;
            }
            
            for(long iOrder=0; iOrder<lSavedNum; iOrder++)  //  스택 메모리 한계로 인해서 한 무리의 Promotions를 한 번에 하나의 Chunk에 담아서 처리
            {
                bool bChanged = false;
                
                if((strcmp(Promotions[iOrder].ClassAxis, caClassAxis)) || \
                        (strcmp(Promotions[iOrder].WriterAxis, caWriterAxis)))
                {
                    bChanged = true;
                    
                    printf("\n          Current promotion number is %ld/%ld.\n", \
                            lPromotionRecNum, lTotalPromotionNum);

                    for(int i = 0; i < Promotions[iOrder].ArtIDsNum; i++)
                    {
                        int iClassIdx = 0, iWriterIdx = 0;
                        double dTmpArticleEmb[EMB_SIZE] = {0};

                        GetClassIdxWriterIdxEmbFromArtID(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                                caaClassAxis, caaWriterAxis, iTotalClassNum, iTotalWriterNum, \
                                Promotions[iOrder].ArticleIDs[i], &iClassIdx, &iWriterIdx, dTmpArticleEmb);

                        if(iClassIdx>=0) dNormalClass[iClassIdx]++;
                        if(iWriterIdx>=0) dNormalWriter[iWriterIdx]++;
                    }

                    //  최종적으로 도출할 dNormalClass, dNormalWriter 계산
                    Normalize(dNormalClass, iTotalClassNum);
                    Normalize(dNormalWriter, iTotalWriterNum);

                    //  최종적으로 도출한 dNormalClass, dNormalWriter 출력
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n        iClass: \n");
                    for(int i=0; i<iTotalClassNum; i++)
                    {
                        printf("%s, ", caaClassAxis[i]);
                    }
                    printf("\n");
                    for(int j=0; j<iTotalClassNum; j++)
                    {
                        printf("%lf, ", dNormalClass[j]);
                    }
                    printf("\n");
                    printf("\n        iWriter: \n");
                    for(int i=0; i<iTotalWriterNum; i++)
                    {
                        printf("%s, ", caaWriterAxis[i]);
                    }
                    printf("\n");
                    for(int j=0; j<iTotalWriterNum; j++)
                    {
                        printf("%lf, ", dNormalWriter[j]);
                    }
                    printf("\n");

                    //  최종적으로 도출한 dArticleEmb, dQueryEmb, dNormalClass, dNormalWriter 저장
                    UpdateClassWriterInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, caClassAxis, caWriterAxis, dNormalClass, dNormalWriter, \
                            iTotalClassNum, iTotalWriterNum);
                }
                
                
                char caPromoRegDate[READ_BUF_SIZE] = {0}, caPromoStartDate[READ_BUF_SIZE] = {0}, caPromoEndDate[READ_BUF_SIZE] = {0};
                GetDatesByPromoID(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                        "naeil_wms_db.WMS_ADBANNER", "BNNR_SEQ", Promotions[iOrder].PromotionID, \
                        caPromoRegDate, READ_BUF_SIZE, caPromoStartDate, \
                        READ_BUF_SIZE, caPromoEndDate, READ_BUF_SIZE);
                time_t tPromoRegDate, tPromoStartDate, tPromoEndDate;
                int64_t mtPromoRegDate, mtPromoStartDate, mtPromoEndDate;
                tPromoRegDate = DatetimeStrToLocalTime(caPromoRegDate);   //For MySQL Datetime Obj 
                tPromoStartDate = DatetimeStrToLocalTime(caPromoStartDate);   //For MySQL Datetime Obj 
                tPromoEndDate = DatetimeStrToLocalTime(caPromoEndDate);   //For MySQL Datetime Obj 
                // MongoDB는 밀리초 단위
                mtPromoRegDate = (int64_t)tPromoRegDate * 1000;
                mtPromoStartDate = (int64_t)tPromoStartDate * 1000;
                mtPromoEndDate = (int64_t)tPromoEndDate * 1000;
                
                if(Promotions[iOrder].RegisteredDate != mtPromoRegDate)
                {
                    bChanged = true;
                    
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n        RegisteredDate: %s\n", caPromoRegDate);
                    
                    UpdateRegDateInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, mtPromoRegDate);
                }
                
                if(Promotions[iOrder].StartDate != mtPromoStartDate)
                {
                    bChanged = true;
                    
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n        StartDate: %s\n", caPromoStartDate);
                    
                    UpdateStartDateInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, mtPromoStartDate);                    
                }
                
                if(Promotions[iOrder].EndDate != mtPromoEndDate)
                {
                    bChanged = true;
                    
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n        EndDate: %s\n", caPromoEndDate);
                    
                    UpdateEndDateInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, mtPromoEndDate);                    
                }

                
                if(IsAllZero(Promotions[iOrder].ArticleEmbedding, EMB_SIZE) && (Promotions[iOrder].ArtIDsNum != 0))
                {
                    bChanged = true;
                    
                    for(int i = 0; i < Promotions[iOrder].ArtIDsNum; i++)
                    {
                        int iClassIdx = 0, iWriterIdx = 0;
                        double dTmpArticleEmb[EMB_SIZE] = {0};

                        GetClassIdxWriterIdxEmbFromArtID(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                                caaClassAxis, caaWriterAxis, iTotalClassNum, iTotalWriterNum, \
                                Promotions[iOrder].ArticleIDs[i], &iClassIdx, &iWriterIdx, dTmpArticleEmb);

                        for(int j=0; j<EMB_SIZE; j++)
                        {
                            dArticleEmb[j] += dTmpArticleEmb[j];
                        }
                    }

                    //  최종적으로 도출한 dArticleEmb 출력
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n      dArticleEmb: \n");
                    for(int j=0; j<EMB_SIZE; j++)
                    {
                        printf("%lf, ", dArticleEmb[j]);
                    }
                    printf("\n");
                    
                    UpdateArticleEmbInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, dArticleEmb, EMB_SIZE);
                }
                
                if(IsAllZero(Promotions[iOrder].QueryEmbedding, EMB_SIZE) && (Promotions[iOrder].QueryKeywordsNum != 0))
                {
                    bChanged = true;
                    
                    for(int i = 0; i < Promotions[iOrder].QueryKeywordsNum; i++)
                    {
                        double dTmpQueryEmb[EMB_SIZE] = {0};
                        
                        GetEmbFromText(Promotions[iOrder].QueryKeywords[i], dTmpQueryEmb, false);   
                        
                        for(int j=0; j<EMB_SIZE; j++)
                        {
                            dQueryEmb[j] += dTmpQueryEmb[j];
                        }                        
                    }

                    //  최종적으로 도출한 dQueryEmb 출력
                    printf("\n        PromotionID: %s\n", Promotions[iOrder].PromotionID);
                    printf("\n        dQueryEmb: \n");
                    for(int j=0; j<EMB_SIZE; j++)
                    {
                        printf("%lf, ", dQueryEmb[j]);
                    }
                    printf("\n");
                    
                    UpdateQueryEmbInDocuByPromoID(BigData_Promotions_collection, Promotions[iOrder].PromotionID, \
                            Promotions[iOrder]._id, dQueryEmb, EMB_SIZE);                    
                }

                                
                fptr = fopen("PromotionAnalyzer.result", "wt");
                fprintf(fptr, "%ld", lPromotionRecNum);
                fclose(fptr);
                if(!bChanged)
                {
                    printf("\n      The analysis of promotions[%ld/%ld] is skipped.\n", \
                            lPromotionRecNum, lTotalPromotionNum);                    
                }

                ////////////////////////////////////////////////////////////////////////////////////////////////////
                //  핵심 변수 메모리 초기화 : 시작
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                for(int j=0; j<iTotalClassNum; j++)
                {
                    dNormalClass[j] = 0;
                }
                for(int j=0; j<iTotalWriterNum; j++)
                {
                    dNormalWriter[j] =  0;
                }
                for(int j=0; j<EMB_SIZE; j++)
                {
                    dArticleEmb[j] = dQueryEmb[j] = 0;
                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                //  핵심 변수 메모리 초기화 : 끝
                ////////////////////////////////////////////////////////////////////////////////////////////////////                    

                lPromotionRecNum++;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 메모리 Clear : 시작
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        for(int i=0; i<iTotalClassNum; i++)
        {
            free(caaClassAxis[i]);
        }
        for(int i=0; i<iTotalWriterNum; i++)
        {
            free(caaWriterAxis[i]);
        }
        free(caaClassAxis);
        free(caaWriterAxis);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 메모리 Clear : 끝
        ////////////////////////////////////////////////////////////////////////////////////////////////////

        lPromotionAnalyzerLoopNum++;
    } while (true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 메모리 Clear : 시작    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    mongoc_collection_destroy (BigData_Promotions_collection);
    mongoc_database_destroy (BigData_database);
    mongoc_uri_destroy (BigData_uri);
    mongoc_client_destroy (BigData_client);
    mongoc_cleanup ();

    mysql_close(DB_conn);
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 메모리 Clear : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}

