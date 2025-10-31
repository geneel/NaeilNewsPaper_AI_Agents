/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * Copyright
 * File:   ArticleAnalyzer.cpp
 * Author: 이해성, geneel@me.com; (주)내일이비즈; (주)내일신문
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




#define TEXT_SIZE 6000 //한글을 기준으로 한 글자수
#define EMB_SIZE 768 //google-vertex AI에서 TextEmbedding을 실행한 결과 벡터의 차원 갯수
#define STR_SIZE 2048
#define NAME_SIZE 256
#define READ_BUF_SIZE 64
//#define MANUAL_DEBUG 1


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


int GetAxesByArtID(mongoc_collection_t* pmgcolThisCollection, char* caArtID, \
        char* caClassAxis, char* caWriterAxis)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr=NULL;
    Document ThisDocument;


    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = BCON_NEW("ArticleID", BCON_UTF8(caArtID));
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
            caClassAxis[0] = caWriterAxis[0] = '\0';
            return 400;
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

int InsertDocuInArticlesCollection(mongoc_collection_t* pmgcolThisCollection, \
        char* ArticleID, char* RegisteredDate, char* ClassAxis, char* WriterAxis, \
        double* NormalClass, int iNormalClassSize, double* NormalWriter, int iNormalWriterSize, \
        double* ArticleAccumEmbedding, int iArticleAccumEmbeddingSize, double* QueryAccumEmbedding, int iQueryAccumEmbeddingSize)
{
    bson_t *pThisInsert;
    bson_error_t ThisError;
    bool bOK=false;

    pThisInsert = bson_new();
    if (!pThisInsert) {
        fprintf(stderr, "bson_new failed in InsertDocuInArticlesCollection()\n");
        return 301;
    }

    if (!BSON_APPEND_UTF8(pThisInsert, "ArticleID", ArticleID)) {
        fprintf(stderr, "append ArticleID failed in InsertDocuInArticlesCollection()\n");
        return 302;
    }
    
    time_t tRegisteredDate;
    tRegisteredDate = DatetimeStrToLocalTime(RegisteredDate);   //For MySQL Datetime Obj    
    // MongoDB는 밀리초 단위
    long mtRegisteredDate = (long)tRegisteredDate * 1000;
    if (!BSON_APPEND_DATE_TIME(pThisInsert, "RegisteredDate", mtRegisteredDate)) {
        fprintf(stderr, "append RegisteredDate failed in InsertDocuInArticlesCollection()\n");
        return 302;
    }
    
    if (!BSON_APPEND_UTF8(pThisInsert, "ClassAxis", ClassAxis)) {
        fprintf(stderr, "append ClassAxis failed in InsertDocuInArticlesCollection()\n");
        return 303;
    }
    if (!BSON_APPEND_UTF8(pThisInsert, "WriterAxis", WriterAxis)) {
        fprintf(stderr, "append WriterAxis failed in InsertDocuInArticlesCollection()\n");
        return 304;
    }

    if (!append_double_array(pThisInsert, "NormalClass", NormalClass, iNormalClassSize)) {
        fprintf(stderr, "append array NormalClass failed in InsertDocuInArticlesCollection()\n");
        return 307;
    }
    if (!append_double_array(pThisInsert, "NormalWriter", NormalWriter, iNormalWriterSize)) {
        fprintf(stderr, "append array NormalWriter failed in InsertDocuInArticlesCollection()\n");
        return 308;
    }
    if (!append_double_array(pThisInsert, "ArticleAccumEmbedding", ArticleAccumEmbedding, iArticleAccumEmbeddingSize)) {
        fprintf(stderr, "append array ArticleAccumEmbedding failed in InsertDocuInArticlesCollection()\n");
        return 309;
    }
    if (!append_double_array(pThisInsert, "QueryAccumEmbedding", QueryAccumEmbedding, iQueryAccumEmbeddingSize)) {
        fprintf(stderr, "append array QueryAccumEmbedding failed in InsertDocuInArticlesCollection()\n");
        return 310;
    }

    bOK = mongoc_collection_insert_one(pmgcolThisCollection, pThisInsert, NULL, NULL, &ThisError);
    if (!bOK) {
        fprintf(stderr, "insert_one failed in InsertDocuInArticlesCollection(): %s\n", ThisError.message);
        return 311;
    }
    
    printf("\nSuccess : %s document for Articles_Collection is inserted\n", ArticleID);
    return 0;
}



int main(int argc, char** argv) {
    long lSavedArticleRecNum = 0;

    char caBigDataServerIP[READ_BUF_SIZE];
    memset(caBigDataServerIP, 0, sizeof(caBigDataServerIP));
    char caDBServerIP[READ_BUF_SIZE];
    memset(caDBServerIP, 0, sizeof(caDBServerIP));
    char caSavedArticleRecNum[READ_BUF_SIZE];
    memset(caSavedArticleRecNum, 0, sizeof(caSavedArticleRecNum));

    char BigData_uri_string[STR_SIZE];
    memset(BigData_uri_string, 0, sizeof(BigData_uri_string));
    mongoc_uri_t *BigData_uri = 0;
    mongoc_client_t *BigData_client = 0;
    mongoc_database_t *BigData_database = 0;
    mongoc_collection_t *BigData_Events_collection, *BigData_Visitors_collection, *BigData_Articles_collection;
    bson_error_t BigData_error = {0};
    BigData_Events_collection = BigData_Visitors_collection = BigData_Articles_collection = 0;

//    mongoc_cursor_t *pEventCursor, *pVisitorCursor, *pAnalCursor;
//    bson_t *pEventQuery, *pEventOpts, *pVisitorQuery, *pVisitorOpts, *pAnalQuery, *pAnalOpts;
//    const bson_t *pEventDoc, *pVisitorDoc, *pAnalDoc;
//    char *pEventStr, *pVisitorStr, *pAnalStr;
//    Document EventDocument, VisitorDocument, AnalDocument;

    MYSQL* DB_conn, DB_connection;
    DB_conn = 0; 
    DB_connection = {0};
    MYSQL_RES* DB_result = 0;
    MYSQL_ROW DB_row; DB_row = {0};
    char DB_HOST[NAME_SIZE];
    memset(DB_HOST, 0, sizeof(DB_HOST));    //호스트명
    char DB_USER[NAME_SIZE];
    memset(DB_USER, 0, sizeof(DB_USER));    //사용자명
    char DB_PASS[NAME_SIZE];
    memset(DB_PASS, 0, sizeof(DB_PASS));    //비밀번호
    char DB_NAME[NAME_SIZE];
    memset(DB_NAME, 0, sizeof(DB_NAME));    //DB명
    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 시작
    ////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MANUAL_DEBUG
    printf("This is starting point.\n");
#endif
    FILE *fptr = fopen("ArticleAnalyzerSetting.set", "rt");
    if(fptr==NULL)
    {
        printf("\nArticleAnalyzerSetting.set File should be exist.\n");
        return 201;
    }

    int rdcnt = 0;
    rdcnt = fscanf(fptr, "%s", caBigDataServerIP);            
    if(!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong BigDataServerIP in ArticleAnalyzerSetting.set\n");
        return 202;
    }

    rdcnt = fscanf(fptr, "%s", caDBServerIP);            
    if(!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong DBServerIP in ArticleAnalyzerSetting.set\n");
        return 203;
    }
    
    rdcnt = fscanf(fptr, "%s", caSavedArticleRecNum);
    if(!rdcnt)
    {
        fclose(fptr);
        printf("Wrong article order number in ArticleAnalyzerSetting.set\n");
        return 212;
    }
    lSavedArticleRecNum = atol(caSavedArticleRecNum);

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
    mongoc_client_set_appname (BigData_client, "ArticleAnalyzer");
    /*
    * Get a handle on the database "db_name" and collection "coll_name"
    */
    BigData_database = mongoc_client_get_database (BigData_client, "WwwNaeilCom2023");
    printf("\nSuccess : BigDataSvr-DB is connected\n");
    BigData_Events_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Events");
    printf("\nSuccess : BigDataSvr-EventsCollection is connected\n");
    BigData_Visitors_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Visitors");
    printf("\nSuccess : BigDataSvr-VisitorsCollection is connected\n");
    BigData_Articles_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Articles");
    printf("\nSuccess : BigDataSvr-ArticlesCollection is connected\n");

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
 
    
    
    bool bIsThisArticleFirstTurn = true;
    long lArticleAnalyzerLoopNum = 0;
    do
    {
        printf("\n  Start ArticleAnalyzer-Loop Number is %ld\n", lArticleAnalyzerLoopNum);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 선언 : 시작
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        long iTotalClassNum, iTotalWriterNum;
        iTotalClassNum = iTotalWriterNum = 0;
        GetTotalRecNum(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, "naeil_wms_db.WMS_CODE_CLASS", \
                &iTotalClassNum);
        printf("\n  Total classes number is %d\n", iTotalClassNum);
        GetTotalRecNum(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, "naeil_wms_db.WMS_USER", \
                &iTotalWriterNum);
        printf("\n  Total writers number is %d\n", iTotalWriterNum);

        double dArticleEmb[EMB_SIZE], dQueryEmb[EMB_SIZE];
        char **caaClassAxis, **caaWriterAxis;
        caaClassAxis = caaWriterAxis = 0;
        char caClassAxis[TEXT_SIZE], caWriterAxis[TEXT_SIZE];
        memset(caClassAxis, 0, sizeof(caClassAxis));
        memset(caWriterAxis, 0, sizeof(caWriterAxis));
        int iClass[iTotalClassNum], iWriter[iTotalWriterNum];
        double dNormalClass[iTotalClassNum], dNormalWriter[iTotalWriterNum];
        
        for(int j=0; j<iTotalClassNum; j++)
        {
            dNormalClass[j] = iClass[j] = 0;
        }
        for(int j=0; j<iTotalWriterNum; j++)
        {
            dNormalWriter[j] = iWriter[j] = 0;
        }
        for(int j=0; j<EMB_SIZE; j++)
        {
            dArticleEmb[j] = dQueryEmb[j] = 0;
        }

        caaClassAxis = (char**)malloc(sizeof(char*) * iTotalClassNum);
        for(int i=0; i<iTotalClassNum; i++)
        {
            caaClassAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
            memset(caaClassAxis[i], 0, READ_BUF_SIZE);
        }
        caaWriterAxis = (char**)malloc(sizeof(char*) * iTotalWriterNum);
        for(int i=0; i<iTotalWriterNum; i++)
        {
            caaWriterAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
            memset(caaWriterAxis[i], 0, READ_BUF_SIZE);
        }
        
        GetAxis(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                "CODE_ID", "naeil_wms_db.WMS_CODE_CLASS", \
                iTotalClassNum, caaClassAxis);
        printf("    iClass: \n");
        for(int i=0; i<iTotalClassNum; i++)
        {
            int len = strlen(caClassAxis);
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
            int len = strlen(caWriterAxis);
            sprintf(caWriterAxis + len, "%s, ", caaWriterAxis[i]);
            printf("%s, ", caaWriterAxis[i]);
        }
        printf("\n");
        
        long lTotalArticleNum, lArticleRecNum;
        lTotalArticleNum = lArticleRecNum = 0;
        GetTotalRecNum(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, "naeil_wms_db.WMS_ARTICLE", \
                &lTotalArticleNum);
        printf("\n  Total articles number is %ld.\n", lTotalArticleNum);
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        //  핵심 변수 선언 : 끝
        ////////////////////////////////////////////////////////////////////////////////////////////////////

        if(bIsThisArticleFirstTurn) //  프로그램이 시작될 때 ArticleAnalyzerSetting.set에서 읽어 온 값으로써 해당 Article에서 작업을 시작하라는 의미
        {
            lArticleRecNum = lSavedArticleRecNum;
            bIsThisArticleFirstTurn = false;
        }
        else
        {
            lArticleRecNum = 0;
        }
        
        char caSQL[STR_SIZE]={0};
        char caArticleID[READ_BUF_SIZE]={0};
        char caRegisteredDate[READ_BUF_SIZE]={0};
        char caQuery[TEXT_SIZE]={0};

        sprintf(caSQL, "SELECT ART_ID, REG_DT, SUB_TITLE FROM naeil_wms_db.WMS_ARTICLE LIMIT 10000000 OFFSET %ld", lArticleRecNum);
        if(mysql_query(DB_conn, caSQL) == 0)
        {
            DB_result = mysql_store_result(DB_conn);
            while((DB_row = mysql_fetch_row(DB_result)) != NULL)
            {
                if (DB_row[0]) strcpy(caArticleID, DB_row[0]); else caArticleID[0] = '\0';
                if (DB_row[1]) strcpy(caRegisteredDate, DB_row[1]); else caRegisteredDate[0] = '\0';
                if (DB_row[2]) strcpy(caQuery, DB_row[2]); else caQuery[0] = '\0';
                printf("\n          Current articles number is %ld/%ld.\n", lArticleRecNum, lTotalArticleNum);

                char caSavedClassAxis[TEXT_SIZE] = {0};
                char caSavedWriterAxis[TEXT_SIZE] = {0};
                GetAxesByArtID(BigData_Articles_collection, caArticleID, caSavedClassAxis, caSavedWriterAxis);
//                printf("caSavedClassAxis = %s\n, caClassAxis = %s\n, caSavedWriterAxis = %s\n, caWriterAxis = %s\n", \
                        caSavedClassAxis, caClassAxis, caSavedWriterAxis, caWriterAxis);
                if((strcmp(caSavedClassAxis, caClassAxis))||(strcmp(caSavedWriterAxis, caWriterAxis)))
                {
                    int iClassIdx, iWriterIdx;
                    iClassIdx = iWriterIdx = 0;
                    if(strcmp(caArticleID, ""))
                    {
                        GetClassIdxWriterIdxEmbFromArtID(&DB_connection, DB_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, \
                                caaClassAxis, caaWriterAxis, iTotalClassNum, iTotalWriterNum, \
                                caArticleID, &iClassIdx, &iWriterIdx, dArticleEmb);
                        GetEmbFromText(caQuery, dQueryEmb, false);                        
                        if(iClassIdx>=0) iClass[iClassIdx]++;
                        if(iWriterIdx>=0) iWriter[iWriterIdx]++;
                    }
                    else
                    {
                        GetEmbFromText(caQuery, dQueryEmb, false);                        
                    }

                    //  최종적으로 도출할 dArticleEmb, dQueryEmb, dNormalClass, dNormalWriter 계산
                    for(int i=0; i<iTotalClassNum; i++)
                    {
                        dNormalClass[i] = iClass[i];
                    }
                    for(int i=0; i<iTotalWriterNum; i++)
                    {
                        dNormalWriter[i] = iWriter[i];
                    }
                    Normalize(dNormalClass, iTotalClassNum);
                    Normalize(dNormalWriter, iTotalWriterNum);

                    //  최종적으로 도출한 dArticleEmb, dQueryEmb, dNormalClass, dNormalWriter 출력
                    printf("\n        ART_ID: %s\n", caArticleID);
                    printf("\n        REG_DT: %s\n", caRegisteredDate);
                    printf("\n      dArticleEmb: \n");
                    for(int j=0; j<EMB_SIZE; j++)
                    {
                        printf("%lf, ", dArticleEmb[j]);
                    }
                    printf("\n");
                    printf("\n        dQueryEmb: \n");
                    for(int j=0; j<EMB_SIZE; j++)
                    {
                        printf("%lf, ", dQueryEmb[j]);
                    }
                    printf("\n");
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
                    InsertDocuInArticlesCollection(BigData_Articles_collection, \
                            caArticleID, caRegisteredDate, caClassAxis, caWriterAxis, \
                            dNormalClass, iTotalClassNum, dNormalWriter, iTotalWriterNum, \
                            dArticleEmb, EMB_SIZE, dQueryEmb, EMB_SIZE);

                    fptr = fopen("ArticleAnalyzer.result", "wt");
                    fprintf(fptr, "%ld", lArticleRecNum);
                    fclose(fptr);                    
                }
                else
                {
                    fptr = fopen("ArticleAnalyzer.result", "wt");
                    fprintf(fptr, "%ld", lArticleRecNum);
                    fclose(fptr);
                    printf("\n      The analysis of article[%ld/%ld] is skipped.\n", \
                            lArticleRecNum, lTotalArticleNum);                                        
                }

                ////////////////////////////////////////////////////////////////////////////////////////////////////
                //  핵심 변수 메모리 초기화 : 시작
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                for(int j=0; j<iTotalClassNum; j++)
                {
                    dNormalClass[j] = iClass[j] = 0;
                }
                for(int j=0; j<iTotalWriterNum; j++)
                {
                    dNormalWriter[j] = iWriter[j] = 0;
                }
                for(int j=0; j<EMB_SIZE; j++)
                {
                    dArticleEmb[j] = dQueryEmb[j] = 0;
                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                //  핵심 변수 메모리 초기화 : 끝
                ////////////////////////////////////////////////////////////////////////////////////////////////////

                lArticleRecNum++;
            }
            mysql_free_result(DB_result);
        }
        else
        {
            // 에러
            char caError[READ_BUF_SIZE] = {0};
            int iErrno = mysql_errno(DB_conn);
            strcpy(caError, mysql_error(DB_conn));
            printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);

            if(iErrno == 2013)
            {
                mysql_init(&DB_connection);
                DB_conn = mysql_real_connect(&DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
            }
            continue;
        }
        caSQL[0] = '\0';
                
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

        lArticleAnalyzerLoopNum++;
    } while (true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 메모리 Clear : 시작    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    mongoc_collection_destroy (BigData_Events_collection);
    mongoc_collection_destroy (BigData_Visitors_collection);
    mongoc_collection_destroy (BigData_Articles_collection);
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

