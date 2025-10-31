/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * Copyright
 * File:   PromotionRecommender.cpp
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


#define TIME_SLOT_SIZE 2048
#define TEXT_SIZE 6000 //한글을 기준으로 한 글자수
#define EMB_SIZE 768 //google-vertex AI에서 TextEmbedding을 실행한 결과 벡터의 차원 갯수
#define STR_SIZE 2048
#define NAME_SIZE 256
#define READ_BUF_SIZE 64
#define MAX_DISTANCE 100
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
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include <curl/curl.h>



using namespace rapidjson;
using namespace std;



typedef struct
{
    char cid[READ_BUF_SIZE], ClassAxis[TEXT_SIZE], WriterAxis[TEXT_SIZE];
    int TotalEventSize, ClassAxisDim, WriterAxisDim;
    double Period[TIME_SLOT_SIZE/2], Acceleration[TIME_SLOT_SIZE], \
            NormalClass[EMB_SIZE], NormalWriter[EMB_SIZE], \
            ArticleAccumEmbedding[EMB_SIZE], QueryAccumEmbedding[EMB_SIZE];    
} AnalDocu;

typedef struct
{
    char PromotionID[READ_BUF_SIZE], ClassAxis[TEXT_SIZE], WriterAxis[TEXT_SIZE];
    int64_t RegisteredDate, StartDate, EndDate;
    int ClassAxisDim, WriterAxisDim;
    double NormalClass[EMB_SIZE], NormalWriter[EMB_SIZE], \
            ArticleAccumEmbedding[EMB_SIZE], QueryAccumEmbedding[EMB_SIZE], \
            Period[TIME_SLOT_SIZE/2], Acceleration[TIME_SLOT_SIZE];    
    double ClassDistance, WriterDistance, ArticleDistance, QueryDistance, \
            PeriodDistance, AccelDistance;
    double TotalDistance;
} PromotionDocu;


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
        strncpy(cass, "00", 3-1);
        cass[3-1] = '\0';
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

int GetCidByMemberID(mongoc_collection_t* pmgcolThisCollection, mongoc_collection_t* pmgcolThatCollection, \
        mongoc_collection_t* pmgcolAnotherCollection, char* caMemberID, char* caCID)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    long lEventSize[TIME_SLOT_SIZE], lInAnals[TIME_SLOT_SIZE];
    char caTmpCID[TIME_SLOT_SIZE][NAME_SIZE];
    int iCounter = 0;
    long lMaxVal = 0;
    long lMaxIdx = 0;

    for(int i=0; i<TIME_SLOT_SIZE; i++)
    {
        lEventSize[i] = 0;
        lInAnals[i] = 0;
        caTmpCID[i][0] = '\0';
    }
    
    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = BCON_NEW ("MemberID", BCON_UTF8(caMemberID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    while(mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto cid = ThisDocument.FindMember("cid");
        if((cid != ThisDocument.MemberEnd())&&(cid->value.IsString()))
        {
            strncpy(caTmpCID[iCounter], cid->value.GetString(), NAME_SIZE-1);
            caTmpCID[iCounter][NAME_SIZE-1] = '\0';
        }
        GetRecordNumByCID(pmgcolThatCollection, caTmpCID[iCounter], &(lEventSize[iCounter]));
        GetRecordNumByCID(pmgcolAnotherCollection, caTmpCID[iCounter], &(lInAnals[iCounter]));

        bson_free(pThisStr);
        iCounter++;
        
        if(iCounter > TIME_SLOT_SIZE) break;
    }
    
    lMaxVal = lEventSize[0] * lInAnals[0];
    for(long i=1; i<TIME_SLOT_SIZE; i++)
    {
        if((lEventSize[i] * lInAnals[i]) > lMaxVal)
        {
            lMaxVal = lEventSize[i] * lInAnals[i];
            lMaxIdx = i;
        }
    }
    
    if(lInAnals[lMaxIdx] != 0)
    {
        strncpy(caCID, caTmpCID[lMaxIdx], NAME_SIZE);
        caCID[NAME_SIZE-1] = '\0';        
    }
    else
    {
        strcpy(caCID, "");
    }

    mongoc_cursor_destroy (pThisCursor);    
    return 0;    
}

int GetAliasedCidByCid(mongoc_collection_t* pmgcolThisCollection, char* caOneCID, char* caTwoCID)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    
    caTwoCID[0] = '\0';

    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = BCON_NEW ("cid01", BCON_UTF8(caOneCID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    while(mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto cid02 = ThisDocument.FindMember("cid02");
        if((cid02 != ThisDocument.MemberEnd())&&(cid02->value.IsString()))
        {
            strncpy(caTwoCID, cid02->value.GetString(), NAME_SIZE-1);
            caTwoCID[NAME_SIZE-1] = '\0';
        }        

        bson_free(pThisStr);
        break;
    }
    if(strcmp(caTwoCID, "") == 0) strcpy(caTwoCID, caOneCID);

    mongoc_cursor_destroy (pThisCursor);    
    return 0;    
}

int GetMemberIDByCid(mongoc_collection_t* pmgcolThisCollection, char* caCID, char* caMemberID)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;

    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    pThisQuery = BCON_NEW ("cid", BCON_UTF8(caCID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
//    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    while(mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto MemberID = ThisDocument.FindMember("MemberID");
        if((MemberID != ThisDocument.MemberEnd())&&(MemberID->value.IsString()))
        {
            strncpy(caMemberID, MemberID->value.GetString(), NAME_SIZE-1);
            caMemberID[NAME_SIZE-1] = '\0';
        }        

        bson_free(pThisStr);
        break;
    }

    mongoc_cursor_destroy (pThisCursor);    
    return 0;    
}


// 특정 필드 이름을 받아 배열 크기와 값을 출력하는 함수
void print_double_array(const bson_t *doc, const char *field_name, double* val, int* count) {
    *count = 0;
    
    bson_iter_t iter;
    if (bson_iter_init_find(&iter, doc, field_name) && BSON_ITER_HOLDS_ARRAY(&iter)) {
        const uint8_t *array_buf;
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
    } else {
        printf("Field \"%s\" not found or not an array.\n\n", field_name);
    }
}

int GetAnalDocuByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, AnalDocu* pAnal)
{
    pAnal->ClassAxisDim = pAnal->WriterAxisDim = 0;
    
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;

    pThisQuery = BCON_NEW("cid", BCON_UTF8(caCID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = bson_new();
    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    //pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    while(mongoc_cursor_next(pThisCursor, &pThisDoc))
    {            
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto cid = ThisDocument.FindMember("cid");
        if((cid != ThisDocument.MemberEnd())&&(cid->value.IsString()))
        {
            strncpy(pAnal->cid, cid->value.GetString(), READ_BUF_SIZE-1);
            pAnal->cid[READ_BUF_SIZE-1] = '\0';
        }        
        auto ClassAxis = ThisDocument.FindMember("ClassAxis");
        if((ClassAxis != ThisDocument.MemberEnd())&&(ClassAxis->value.IsString()))
        {
            strncpy(pAnal->ClassAxis, ClassAxis->value.GetString(), TEXT_SIZE - 1);
            pAnal->ClassAxis[TEXT_SIZE - 1] = '\0';
        }        
        auto WriterAxis = ThisDocument.FindMember("WriterAxis");
        if((WriterAxis != ThisDocument.MemberEnd())&&(WriterAxis->value.IsString()))
        {
            strncpy(pAnal->WriterAxis, WriterAxis->value.GetString(), TEXT_SIZE - 1);
            pAnal->WriterAxis[TEXT_SIZE - 1] = '\0';
        }        

        auto TotalEventSize = ThisDocument.FindMember("TotalEventSize");
        if((TotalEventSize != ThisDocument.MemberEnd())&&(TotalEventSize->value.IsInt64()))
        {
            pAnal->TotalEventSize = TotalEventSize->value.GetInt64();
        }        

        int iDummy;
        print_double_array(pThisDoc, "Period", pAnal->Period, &iDummy);
        print_double_array(pThisDoc, "Acceleration", pAnal->Acceleration, &iDummy);
        print_double_array(pThisDoc, "NormalClass", pAnal->NormalClass, &(pAnal->ClassAxisDim));
        print_double_array(pThisDoc, "NormalWriter", pAnal->NormalWriter, &(pAnal->WriterAxisDim));
        print_double_array(pThisDoc, "ArticleAccumEmbedding", pAnal->ArticleAccumEmbedding, &iDummy);
        print_double_array(pThisDoc, "QueryAccumEmbedding", pAnal->QueryAccumEmbedding, &iDummy);

        bson_free(pThisStr);
        break;
    }

    mongoc_cursor_destroy (pThisCursor);    
    return 0;    
}

bool ValidateAnalDocu(AnalDocu* pAnal)
{
    for(int i = 0; i < pAnal->ClassAxisDim; i++)
    {
        if(pAnal->NormalClass[i] > 0) return true;
    }
    for(int i = 0; i < pAnal->WriterAxisDim; i++)
    {
        if(pAnal->NormalWriter[i] > 0) return true;
    }
    for(int i = 0; i < EMB_SIZE; i++)
    {
        if(pAnal->ArticleAccumEmbedding[i] != 0) return true;
    }
    for(int i = 0; i < EMB_SIZE; i++)
    {
        if(pAnal->QueryAccumEmbedding[i] != 0) return true;
    }
    
    return false;
}

int GetTotalPromotionNumByDate(mongoc_collection_t* pmgcolThisCollection, int64_t RegiStartDate, int64_t RegiEndDate, long* TotalPromotionNum)
{
    bson_t *pThisQuery;
    bson_error_t error;

//    count = mongoc_collection_count_documents (pmgcolThisCollection, NULL, NULL, NULL, NULL, &error);
//    pThisQuery = BCON_NEW ("cid", BCON_UTF8(caCID));
//    pThisQuery = BCON_NEW("RegisteredDate", "{",
//                        "$gte", BCON_DATE_TIME(RegiStartDate),
//                        "$lte", BCON_DATE_TIME(RegiEndDate),
//                     "}");

    // (1) 현재 시각 (UTC 기준, 밀리초 단위)
    int64_t NowDate = (int64_t)time(NULL) * 1000;

    // (2) 복합 AND 조건 생성
    // RegisteredDate 범위 + (StartDate <= NowDate <= EndDate)
    pThisQuery = BCON_NEW(
        "$and", "[",
            "{",
                "RegisteredDate", "{",
                    "$gte", BCON_DATE_TIME(RegiStartDate),
                    "$lte", BCON_DATE_TIME(RegiEndDate),
                "}",
            "}",
            "{",
                "StartDate", "{", "$lte", BCON_DATE_TIME(NowDate), "}",
            "}",
            "{",
                "EndDate", "{", "$gte", BCON_DATE_TIME(NowDate), "}",
            "}",
            "{", 
                "InUse", BCON_BOOL(true), 
            "}",
        "]"
    );

    *TotalPromotionNum = (long)mongoc_collection_count_documents(pmgcolThisCollection, pThisQuery, NULL, NULL, NULL, &error);
    bson_destroy (pThisQuery);
    if (*TotalPromotionNum < 0) 
    {
        printf("%s\n", error.message);
        fprintf (stderr, "%s\n", error.message);
        return 401;
    }
    
    return 0;    
}

int GetPromotionsByDate(mongoc_collection_t* pmgcolThisCollection, \
        int64_t RegiStartDate, int64_t RegiEndDate, long TotalPromotionNum, PromotionDocu* pPromotions)
{
    mongoc_cursor_t *pThisCursor;
    bson_t *pThisQuery, *pThisOpts;
    const bson_t *pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    int iCounter=0;

//    pThisQuery = BCON_NEW("cid", BCON_UTF8(caCID));
//    pThisQuery = BCON_NEW("RegisteredDate", "{",
//                        "$gte", BCON_DATE_TIME(RegiStartDate),
//                        "$lte", BCON_DATE_TIME(RegiEndDate),
//                     "}");

    // (1) 현재 시각 (UTC 기준, 밀리초 단위)
    int64_t NowDate = (int64_t)time(NULL) * 1000;

    // (2) 복합 AND 조건 생성
    // RegisteredDate 범위 + (StartDate <= NowDate <= EndDate)
    pThisQuery = BCON_NEW(
        "$and", "[",
            "{",
                "RegisteredDate", "{",
                    "$gte", BCON_DATE_TIME(RegiStartDate),
                    "$lte", BCON_DATE_TIME(RegiEndDate),
                "}",
            "}",
            "{",
                "StartDate", "{", "$lte", BCON_DATE_TIME(NowDate), "}",
            "}",
            "{",
                "EndDate", "{", "$gte", BCON_DATE_TIME(NowDate), "}",
            "}",
            "{", 
                "InUse", BCON_BOOL(true), 
            "}",
        "]"
    );
    
//    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
//    pThisOpts = bson_new();
    //pThisQuery = bson_new();
    //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
    //pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
    //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy (pThisQuery);
    bson_destroy (pThisOpts);
    
    while((mongoc_cursor_next(pThisCursor, &pThisDoc))&&(iCounter < TotalPromotionNum))
    {            
        pPromotions[iCounter].ClassAxisDim = pPromotions[iCounter].WriterAxisDim = 0;

        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto PromotionID = ThisDocument.FindMember("PromotionID");
        if((PromotionID != ThisDocument.MemberEnd())&&(PromotionID->value.IsString()))
        {
            strncpy(pPromotions[iCounter].PromotionID, PromotionID->value.GetString(), READ_BUF_SIZE - 1);
            pPromotions[iCounter].PromotionID[READ_BUF_SIZE - 1] = '\0';
        }        
        auto ClassAxis = ThisDocument.FindMember("ClassAxis");
        if((ClassAxis != ThisDocument.MemberEnd())&&(ClassAxis->value.IsString()))
        {
            strncpy(pPromotions[iCounter].ClassAxis, ClassAxis->value.GetString(), TEXT_SIZE - 1);
            pPromotions[iCounter].ClassAxis[TEXT_SIZE - 1] = '\0';
        }        
        auto WriterAxis = ThisDocument.FindMember("WriterAxis");
        if((WriterAxis != ThisDocument.MemberEnd())&&(WriterAxis->value.IsString()))
        {
            strncpy(pPromotions[iCounter].WriterAxis, WriterAxis->value.GetString(), TEXT_SIZE - 1);
            pPromotions[iCounter].WriterAxis[TEXT_SIZE - 1] = '\0';
        }        

        bson_iter_t iter1;
        if (bson_iter_init_find(&iter1, pThisDoc, "RegisteredDate") &&
            BSON_ITER_HOLDS_DATE_TIME(&iter1)) {
            int64_t millis1 = bson_iter_date_time(&iter1);
            pPromotions[iCounter].RegisteredDate = millis1;   // UTC 밀리초
        }
        bson_iter_t iter2;
        if (bson_iter_init_find(&iter2, pThisDoc, "StartDate") &&
            BSON_ITER_HOLDS_DATE_TIME(&iter2)) {
            int64_t millis2 = bson_iter_date_time(&iter2);
            pPromotions[iCounter].StartDate = millis2;   // UTC 밀리초
        }
        bson_iter_t iter3;
        if (bson_iter_init_find(&iter3, pThisDoc, "EndDate") &&
            BSON_ITER_HOLDS_DATE_TIME(&iter3)) {
            int64_t millis3 = bson_iter_date_time(&iter3);
            pPromotions[iCounter].EndDate = millis3;   // UTC 밀리초
        }
        
        int iDummy;
        print_double_array(pThisDoc, "NormalClass", pPromotions[iCounter].NormalClass, &(pPromotions[iCounter].ClassAxisDim));
        print_double_array(pThisDoc, "NormalWriter", pPromotions[iCounter].NormalWriter, &(pPromotions[iCounter].WriterAxisDim));
        print_double_array(pThisDoc, "ArticleEmbedding", pPromotions[iCounter].ArticleAccumEmbedding, &iDummy);
        print_double_array(pThisDoc, "QueryEmbedding", pPromotions[iCounter].QueryAccumEmbedding, &iDummy);
        print_double_array(pThisDoc, "Period", pPromotions[iCounter].Period, &iDummy);
        print_double_array(pThisDoc, "Acceleration", pPromotions[iCounter].Acceleration, &iDummy);

        bson_free(pThisStr);
        iCounter++;
    }

    mongoc_cursor_destroy (pThisCursor);    
    return 0;        
}


// Normalized Euclidean Distance 계산 함수
int normalized_euclidean_distance(const double *x, const double *y, int N, double* norm) 
{
    if(N==0)
    {
        *norm = 0;
    }
    else
    {
        double sum_sq = 0.0;
        for (int i = 0; i < N; i++) {
            double diff = x[i] - y[i];
            sum_sq += diff * diff;
        }
        double dist = sqrt(sum_sq);
        *norm = dist / sqrt((double)N);  // [0,1] 범위로 정규화
    }
    return 0;
}

int euclidean_distance(const double *x, const double *y, int N, double* norm) 
{
    double sum_sq = 0.0;
    for (int i = 0; i < N; i++) {
        double diff = x[i] - y[i];
        sum_sq += diff * diff;
    }
    *norm = sqrt(sum_sq);
    return 0;
}

// Cosine similarity 계산 함수
int cosine_similarity(const double *x, const double *y, int N, double* cossim) 
{
    double dot = 0.0, norm_x = 0.0, norm_y = 0.0;

    for (int i = 0; i < N; i++) {
        dot     += x[i] * y[i];    // 내적
        norm_x  += x[i] * x[i];    // x의 제곱합
        norm_y  += y[i] * y[i];    // y의 제곱합
    }

    if (norm_x == 0.0 || norm_y == 0.0) {
        // 영벡터일 경우 0 반환 (혹은 NaN 처리)
        *cossim = 0.0;
    }
    else
    {
        *cossim = dot / (sqrt(norm_x) * sqrt(norm_y));
    }

    return 0;
}

// 비교 함수: TotalDistance 오름차순
int compareByTotalDistance(const void* a, const void* b) {
    double distA = ((PromotionDocu*)a)->TotalDistance;
    double distB = ((PromotionDocu*)b)->TotalDistance;

    if (distA < distB) return -1;
    else if (distA > distB) return 1;
    else return 0;
}


int create_json_payload(char *json, char* caCID, char* caMemberID, PromotionDocu* pPromotions, long lMaxReturn) 
{
    sprintf(json, "{\"cid\":\"%s\",\"MEM_ID\":\"%s\",\"BNNR_SEQ\":[", caCID, caMemberID);
    int len = strlen(json);
    
    if((lMaxReturn > 0) && (pPromotions != 0))
    {
        sprintf(json + len, "\"%s\"", pPromotions[0].PromotionID);
        for(long i=1; i<lMaxReturn; i++)
        {
            len = strlen(json);
            sprintf(json + len, ",\"%s\"", pPromotions[i].PromotionID);
        }
    }
    else
    {
        sprintf(json + len, "\"%s\"", "null");
    }
    strncat(json, "]}", 3);
    return 0;
}

void post_json_to_server(const char *url, const char *json_data) 
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl initialization failed\n");
#ifdef MANUAL_DEBUG
        printf("post_json_to_server:          curl initialization failed\n");
#endif
        return;
    }

    CURLcode res;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=UTF-8");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 응답을 stdout에 출력
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

#ifdef MANUAL_DEBUG
    printf(">> Posting JSON to server...\n");
#endif
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "\nPOST failed: %s\n", curl_easy_strerror(res));
        printf("POST failed: %s\n", curl_easy_strerror(res));
    } else {
#ifdef MANUAL_DEBUG
        printf("\n>> POST succeeded.\n");
#endif
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}



int main(int argc, char** argv) {
    if(argc!=7) {
        printf("Usage   :   ./PromotionRecommender cid MEM_ID Start_Date End_Date Max_Return_Num Callback_URL\n");
        printf("Example :   ./PromotionRecommender 7z3kwdmx4zm2bepo N 2024-01-26T09:35:54.134Z 2025-07-20T19:00:12.512Z 10 http://bgds.naeil.com:5000/upload\n");
        return 11;
    }

    char caBigDataServerIP[READ_BUF_SIZE];

    char BigData_uri_string[STR_SIZE];  // "mongodb://localhost:37017"
    mongoc_uri_t *BigData_uri;
    mongoc_client_t *BigData_client;
    mongoc_database_t *BigData_database;
    mongoc_collection_t *BigData_Promotions_collection, *BigData_Visitors_collection, \
            *BigData_Anals_collection, *BigData_Events_collection, *BigData_Alias_collection;
    bson_error_t BigData_error;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 시작
    ////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MANUAL_DEBUG
    printf("This is starting point.\n");
#endif
    FILE *fptr = fopen("/usr/local/bin/PromotionRecommenderSetting.set", "rt");
    if(fptr==NULL)
    {
        printf("\n/usr/local/bin/PromotionRecommenderSetting.set File should be exist.\n");
        return 201;
    }

    int rdcnt;
    rdcnt = fscanf(fptr, "%s", caBigDataServerIP);            
    if(!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong BigDataServerIP in /usr/local/bin/PromotionRecommenderSetting.set\n");
        return 202;
    }

    fclose(fptr);
#ifdef MANUAL_DEBUG
    printf("\nNaeil-CRM Big Data Server(for www.naeil.com) : IP = %s\n", caBigDataServerIP);
#endif

    strncpy(BigData_uri_string, "", STR_SIZE - 1);
    BigData_uri_string[STR_SIZE - 1] = '\0';
    strncat(BigData_uri_string, caBigDataServerIP, sizeof(BigData_uri_string) - strlen(BigData_uri_string) - 1);
    strncat(BigData_uri_string, "", sizeof(BigData_uri_string) - strlen(BigData_uri_string) - 1);
    strncat(BigData_uri_string, "", sizeof(BigData_uri_string) - strlen(BigData_uri_string) - 1);

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
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr is connected\n");
#endif
    /*
    * Create a new client instance
    */
    BigData_client = mongoc_client_new_from_uri (BigData_uri);
    if (!BigData_client) {
        printf("\nError : mongoc_client_new_from_uri\n");
        return EXIT_FAILURE;
    }
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-Client is connected\n");
#endif
    /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples).
    */
    mongoc_client_set_appname (BigData_client, "PromotionRecommender");
    /*
    * Get a handle on the database "db_name" and collection "coll_name"
    */
    BigData_database = mongoc_client_get_database (BigData_client, "WwwNaeilCom2023");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-DB is connected\n");
#endif
    BigData_Promotions_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Promotions");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-PromotionsCollection is connected\n");
#endif
    BigData_Visitors_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Visitors");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-VisitorsCollection is connected\n");
#endif
    BigData_Anals_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Anals");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-AnalsCollection is connected\n");
#endif
    BigData_Events_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Events");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-EventsCollection is connected\n");
#endif
    BigData_Alias_collection = mongoc_client_get_collection (BigData_client, "WwwNaeilCom2023", "Alias");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-AliasCollection is connected\n");
#endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer 접속 : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    char caCID[NAME_SIZE], caAlias[NAME_SIZE], caMemberID[NAME_SIZE], post_url[STR_SIZE];
    char caStartTime[NAME_SIZE], caEndTime[NAME_SIZE], caMaxReturn[READ_BUF_SIZE];
    time_t tmStartTime, tmEndTime;
    int64_t msStartTime, msEndTime;
    AnalDocu AnalDocument;
    long lMaxReturn, lTotalPromotionNum;
    PromotionDocu* pPromotions;
    caCID[0]='\0'; caAlias[0] = '\0'; caMemberID[0]='\0'; post_url[0]='\0';
    caStartTime[0]='\0'; caEndTime[0]='\0'; caMaxReturn[0]='\0';
    tmStartTime=0; tmEndTime=0;
    msStartTime=0; msEndTime=0;
    AnalDocument={0};
    lMaxReturn=0; lTotalPromotionNum=0;
    pPromotions = 0;
     
    strncpy(caAlias, argv[1], NAME_SIZE - 1);
    caAlias[NAME_SIZE - 1] = '\0';
    strncpy(caMemberID, argv[2], NAME_SIZE - 1);
    caMemberID[NAME_SIZE - 1] = '\0';
    strncpy(caStartTime, argv[3], NAME_SIZE - 1);
    caStartTime[NAME_SIZE - 1] = '\0';
    strncpy(caEndTime, argv[4], NAME_SIZE - 1);
    caEndTime[NAME_SIZE - 1] = '\0';
    strncpy(caMaxReturn, argv[5], READ_BUF_SIZE - 1);
    caMaxReturn[READ_BUF_SIZE - 1] = '\0';
    strncpy(post_url, argv[6], STR_SIZE - 1);
    post_url[STR_SIZE - 1] = '\0';
    
    if((!strcmp(caStartTime, "N"))||(!strcmp(caStartTime, "n"))||(!strcmp(caStartTime, "null")))
    {
        strncpy(caStartTime, "1970-01-01T00:00:00.000Z", NAME_SIZE - 1);
        caStartTime[NAME_SIZE - 1] = '\0';
    }
    if((!strcmp(caEndTime, "N"))||(!strcmp(caEndTime, "n"))||(!strcmp(caEndTime, "null")))
    {
        strncpy(caEndTime, "2170-01-01T00:00:00.000Z", NAME_SIZE - 1);
        caEndTime[NAME_SIZE - 1] = '\0';
    }
    if((!strcmp(caMaxReturn, "N"))||(!strcmp(caMaxReturn, "n"))||(!strcmp(caMaxReturn, "null")))
    {
        strncpy(caMaxReturn, "1000", READ_BUF_SIZE - 1);
        caMaxReturn[READ_BUF_SIZE - 1] = '\0';
    }
    tmStartTime = ISO8601StrToLocalTime(caStartTime); tmEndTime = ISO8601StrToLocalTime(caEndTime);
    msStartTime = tmStartTime * 1000; msEndTime = tmEndTime * 1000;
    lMaxReturn = atol(caMaxReturn);
    
    if((!strcmp(caAlias, "N"))||(!strcmp(caAlias, "n"))||(!strcmp(caAlias, "null")))
    {
        if((!strcmp(caMemberID, "N"))||(!strcmp(caMemberID, "n"))||(!strcmp(caMemberID, "null")))
        {
            printf("Error : At least one of [cid, MEM_ID] should not be \'N\'!\n");
            goto cleanup;
        }
        else
        {
            GetCidByMemberID(BigData_Visitors_collection, BigData_Events_collection, \
                    BigData_Anals_collection, caMemberID, caAlias);
            GetAliasedCidByCid(BigData_Alias_collection, caAlias, caCID);
        }
    }
    else
    {
        GetAliasedCidByCid(BigData_Alias_collection, caAlias, caCID);
        GetMemberIDByCid(BigData_Visitors_collection, caCID, caMemberID);
        if(strcmp(caMemberID, "") == 0)
        {
            GetMemberIDByCid(BigData_Visitors_collection, caAlias, caMemberID);
        }
     }

    if(strcmp(caCID, ""))
    {
        GetAnalDocuByCID(BigData_Anals_collection, caCID, &AnalDocument);
        
        if(ValidateAnalDocu(&AnalDocument))
        {
            GetTotalPromotionNumByDate(BigData_Promotions_collection, msStartTime, msEndTime, &lTotalPromotionNum);

            if(lTotalPromotionNum > 0)
            {
                pPromotions = (PromotionDocu*)malloc(lTotalPromotionNum * sizeof(PromotionDocu));
                GetPromotionsByDate(BigData_Promotions_collection, msStartTime, msEndTime, lTotalPromotionNum, pPromotions);

                for(long i; i<lTotalPromotionNum; i++)
                {
                    if((!strcmp(AnalDocument.ClassAxis, pPromotions[i].ClassAxis))&&\
                            (!strcmp(AnalDocument.WriterAxis, pPromotions[i].WriterAxis)))
                    {
                        normalized_euclidean_distance(AnalDocument.NormalClass, pPromotions[i].NormalClass, \
                                AnalDocument.ClassAxisDim, &(pPromotions[i].ClassDistance));
                        normalized_euclidean_distance(AnalDocument.NormalWriter, pPromotions[i].NormalWriter, \
                                AnalDocument.WriterAxisDim, &(pPromotions[i].WriterDistance));
                        normalized_euclidean_distance(AnalDocument.Period, pPromotions[i].Period, \
                                TIME_SLOT_SIZE/2, &(pPromotions[i].PeriodDistance));
                        normalized_euclidean_distance(AnalDocument.Acceleration, pPromotions[i].Acceleration, \
                                TIME_SLOT_SIZE, &(pPromotions[i].AccelDistance));
                        cosine_similarity(AnalDocument.ArticleAccumEmbedding, pPromotions[i].ArticleAccumEmbedding, \
                                EMB_SIZE, &(pPromotions[i].ArticleDistance));
                        if(pPromotions[i].ArticleDistance < 0) pPromotions[i].ArticleDistance = 0;
                        pPromotions[i].ArticleDistance = fabs(pPromotions[i].ArticleDistance - 1);
                        cosine_similarity(AnalDocument.QueryAccumEmbedding, pPromotions[i].QueryAccumEmbedding, \
                                EMB_SIZE, &(pPromotions[i].QueryDistance));
                        if(pPromotions[i].QueryDistance < 0) pPromotions[i].QueryDistance = 0;
                        pPromotions[i].QueryDistance = fabs(pPromotions[i].QueryDistance - 1);
                        pPromotions[i].TotalDistance = pPromotions[i].ClassDistance + \
                                pPromotions[i].PeriodDistance + pPromotions[i].AccelDistance + \
                                pPromotions[i].WriterDistance + pPromotions[i].ArticleDistance + \
                                pPromotions[i].QueryDistance;            
                    }
                    else
                    {
                        pPromotions[i].TotalDistance = MAX_DISTANCE;
                    }
                }
                qsort(pPromotions, lTotalPromotionNum, sizeof(PromotionDocu), compareByTotalDistance);

                if(lTotalPromotionNum < lMaxReturn) lMaxReturn = lTotalPromotionNum;   
            }
            else
            {
                lMaxReturn = 0;
            }            
        }
        else
        {
            lMaxReturn = 0;
        }
    }
    else
    {
        lMaxReturn = 0;
    }

    if(!strcmp(caCID, "")) strncpy(caCID, "null", sizeof(caCID));
    if(!strcmp(caMemberID, "")) strncpy(caMemberID, "null", sizeof(caMemberID));
    char caResult[TEXT_SIZE*300];
    curl_global_init(CURL_GLOBAL_ALL); // curl 초기화
    create_json_payload(caResult, caCID, caMemberID, pPromotions, lMaxReturn);
    printf("\n%s\n\n", caResult);
    if((strcmp(post_url, "N"))&&(strcmp(post_url, "n"))&&(strcmp(post_url, "null")))
    {
        post_json_to_server(post_url, caResult);       
    }
    curl_global_cleanup();

    if(pPromotions) free(pPromotions);
    goto cleanup;
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer 메모리 Clear : 시작    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    cleanup:
    mongoc_collection_destroy (BigData_Promotions_collection);
    mongoc_collection_destroy (BigData_Visitors_collection);
    mongoc_collection_destroy (BigData_Anals_collection);
    mongoc_collection_destroy (BigData_Events_collection);
    mongoc_database_destroy (BigData_database);
    mongoc_uri_destroy (BigData_uri);
    mongoc_client_destroy (BigData_client);
    mongoc_cleanup ();
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer 메모리 Clear : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}

