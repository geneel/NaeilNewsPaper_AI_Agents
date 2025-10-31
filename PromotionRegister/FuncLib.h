#pragma once

#include <afxstr.h>   // CString
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include <windows.h>
#include <bcrypt.h>
#include <iostream>
#include <string>
#include <cstring>

#pragma comment(lib, "bcrypt.lib")

#include "vcpkg_installed\x64-windows\include\mysql\mysql.h"
#include "vcpkg_installed\x64-windows\include\mongoc/mongoc.h"
#include "vcpkg_installed\x64-windows\include\bson/bson.h"
#include "vcpkg_installed\x64-windows\include\rapidjson/document.h"
using namespace rapidjson;

#define MAX_TOKENS  4096   // 최대 단어 개수
#define MIN_TOKENS  128
#define MAX_LENGTH  64    // 단어 최대 길이
#define STR_LENGTH  300000  // 버퍼 길이

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



typedef struct
{
    char Key[MAX_LENGTH], Value[MAX_LENGTH];
} Dictionary;

typedef struct
{
    char caInsertedDate[READ_BUF_SIZE];
    char caPromoID[READ_BUF_SIZE];
    int iArtIDsNum, iSearchKeywordsNum;
    char caaArtIDs[MIN_TOKENS][NAME_SIZE], caaSearchKeywords[MIN_TOKENS][NAME_SIZE];
    char caVisitorPattern[TEXT_SIZE * 10];
} RegiPromoDlgRec;

typedef struct
{
    char caCID[NAME_SIZE], caName[NAME_SIZE], caPhone[NAME_SIZE], \
        caMail[NAME_SIZE], caID[NAME_SIZE], caRecommenders[NAME_SIZE];
} VisitorDocu;

typedef struct
{
    char cid[READ_BUF_SIZE], ClassAxis[TEXT_SIZE], WriterAxis[TEXT_SIZE];
    char RegistedDate[READ_BUF_SIZE];
    int TotalEventSize, ClassAxisDim, WriterAxisDim;
    double Period[TIME_SLOT_SIZE / 2], Acceleration[TIME_SLOT_SIZE], \
        NormalClass[EMB_SIZE], NormalWriter[EMB_SIZE], \
        ArticleAccumEmbedding[EMB_SIZE], QueryAccumEmbedding[EMB_SIZE];
} AnalDocu;

typedef struct
{
    char ArticleID[READ_BUF_SIZE], ClassAxis[TEXT_SIZE], WriterAxis[TEXT_SIZE];
    int64_t RegisteredDate;
    int ClassAxisDim, WriterAxisDim;
    double NormalClass[EMB_SIZE], NormalWriter[EMB_SIZE], \
        ArticleAccumEmbedding[EMB_SIZE], QueryAccumEmbedding[EMB_SIZE];
    double ClassDistance, WriterDistance, ArticleDistance, QueryDistance;
    double TotalDistance;
} ArticleDocu;



// 입력한 CSV형식의 CString을 rslt_count 개의 문자열로 변환
int CStringCSVToMultiArray(CString csData, char words[][MAX_LENGTH], int* rslt_count);

// 입력 문자열에서 모든 공백 제거
void RemoveSpaces(char* str);

// 문자열을 "키:값" 형식에서 Dictionary 구조체로 파싱
int ParseKeyValue(const char* input, Dictionary* dict);

// 입력된 caID와 caPWD가 몽고디비의 Users 컬렉션에 몇 개 존재하는 지를 piRecordNum에 기록
int GetRecordNumByIdPwd(mongoc_collection_t* pmgcolThisCollection, char* caID, char* caPWD, int* piRecordNum);

// 간단히 SHA256으로 해시 (Windows CNG API 사용)
std::string HashPassword(const std::string& password);

// MFC의 CString을 std::string으로 변환
std::string CStringToStdString(const CString& cstr);

// MFC의 CString을 UTF8의 std::string으로 변환
std::string CStringToUtf8(const CString& cstr);

// UTF8의 std::string을 MFC의 CString으로 변환
CString Utf8ToCString(const std::string& utf8str);

// Char Arrapy(CP949) → UTF-8 변환
std::string CharArrayToUTF8(const char* src);

// MFC의 CString을 UTF8의 std::string으로 변환
std::string CStringObjToUTF8(const CString& src);

// UTF-8 → Windows ANSI (CP_ACP) 변환 함수
// in_utf8: 입력 UTF-8 문자열
// out_ansi: 변환 결과를 받을 버퍼
// out_size: 버퍼 크기 (바이트 단위)
void Utf8ToAnsi(const char* in_utf8, char* out_ansi, size_t out_size);

// std::string 변수의 배열을 입력으로 받아서 2차원의 char array로 변환
void ConvertStringArrayToCharArray(char (*dest)[MAX_LENGTH], const std::string* src, int count);

// ---------------------------------------------------------------------------
// 내부 함수: 문자열이 이미 UTF-8 형식인지 검사
// ---------------------------------------------------------------------------
inline bool IsValidUTF8_Win(const char* str);

// ---------------------------------------------------------------------------
// ANSI (CP949 등) → UTF-8 변환
// ---------------------------------------------------------------------------
inline std::string CharArrayToUTF8_Win(const char* ansiStr);

// ---------------------------------------------------------------------------
// UTF-8 → ANSI (CP949 등) 변환
// ---------------------------------------------------------------------------
inline std::string UTF8ToCharArray_Win(const char* utf8Str);

// ---------------------------------------------------------------------------
// 1️⃣ UTF-8 유효성 검사
// ---------------------------------------------------------------------------
bool IsValidUTF8(const char* str);

// ---------------------------------------------------------------------------
// 2️⃣ ANSI → UTF-8 변환 (CP949 → UTF-8)
//     dst_size는 바이트 단위이며, 항상 '\0' 보장됨
// ---------------------------------------------------------------------------
bool CharArrayToUTF8(const char* ansiStr, char* utf8Buf, size_t dst_size);

// ---------------------------------------------------------------------------
// 3️⃣ UTF-8 → ANSI 변환 (UTF-8 → CP949)
// ---------------------------------------------------------------------------
bool UTF8ToCharArray(const char* utf8Str, char* ansiBuf, size_t dst_size);

// 주어진 TB_NAME 테이블에서 caToken값이 caID와 일치하는 레코드의 갯수를 ipTotalNum에 기록
int GetRecNum(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, char* caToken, char* caID, int* ipTotalNum);

// 주어진 배너용 TB_NAME 테이블에서 caToken값이 caID와 일치하는 레코드의 입력일, 시작일, 종료일을 각각 caRegDate, caStartDate, caEndDate에 기록
int GetDatesByPromoID(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, char* caToken, char* caID, \
    char* caRegDate, int iRegDSize, char* caStartDate, int iStartDSize, char* caEndDate, int iEndDSize);

// 주어진 TB_NAME 테이블의 모든 레코드의 갯수를 ipTotalNum에 기록
int GetTotalRecNum(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, \
    long* ipTotalNum);

// 주어진 TB_NAME 테이블의 COL_NAME 컬럼의 모든 값들을 caaAxis[][]에 기록
int GetAxis(MYSQL* DB_connection, MYSQL* DB_conn, char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, \
    char* COL_NAME, char* TB_NAME, \
    int iTotalNum, char** caaAxis, int iStrSize);

// 주어진 ART_ID, caaClassAxis, caaWriterAxis에 대해서 해당 ART_ID가 속한 caaClassAxis, caaWriterAxis 축의 인덱스를 각각 iClassIdx, iWriterIdx에 기록
int GetClassIdxWriterIdxFromArtID(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, \
    char** caaClassAxis, char** caaWriterAxis, \
    int iTotalClassNum, int iTotalWriterNum, char* ART_ID, \
    int* iClassIdx, int* iWriterIdx);

// iTotalNum개의 double 배열 dNormal을 배열 원소중의 최대값이 1이 되도록 정규화
int Normalize(double* dNormal, int iTotalNum);

/* 인덱스 clamp: [0, n-1] 범위로 고정 */
static inline int clamp(int i, int n);

/* 길이 32의 가우시안 커널 생성 (정규화 포함)
 * sigma: 표준편차 (예: 4.0 ~ 6.0 권장; 대략 KLEN/6 ≈ 5.3 근처가 무난)
 * anchor: 커널 기준 인덱스(정수 중심). 짝수 길이에서는 KLEN/2 사용.
 */
static void build_gaussian_kernel(double kernel[KLEN], double sigma);

/* 1D Gaussian Smoothing
 * in  : 입력 신호 (float)
 * out : 출력 신호 (float)
 * n   : 신호 길이
 * sigma: 가우시안 표준편차 = 5.33; /* 길이 32 커널에서 무난한 값 (대략 KLEN/6)
 */
void gaussian_smooth_1d(const double* in, double* out, int n, double sigma);

//  배열 data[]의 시작 인덱스가 0이 아닌 1이라는 사실에 유의
//  따라서 data[]의 크기는 (nn*2 + 1)이다.
void four1(double data[], unsigned long nn, int isign);

//  배열 data[]의 시작 인덱스가 0이 아닌 1이라는 사실에 유의
//  따라서 data[]의 크기는 (nn*2 + 1)이다.
//  반면에 dRslt[]의 크기는 nn이며 시작 인덱스는 0이다.
int PowerSpectralDensity(double data[], int nn, double* dRslt);

// 크기 iInputLen의 입력 데이터 dInput을 크기 iFilterKlen의 dFilter로 iLevel 레벨까지 Wavelet Transform하여 dOutput에 기록
int DWT(double* dFilter, int iFilterKlen, int iLevel, double* dInput, int iInputLen, double* dOutput);

// 크기 iInputLen의 입력 데이터 dInput을 크기 iLPFilterKlen의 dLPFilter와 크기 iHPFilterKlen의 dHPFilter로 iLevel 레벨까지
// Wavelet Transform하여 2차원 데이터 dOutput에 기록
int DWTBank(double* dLPFilter, int iLPFilterKlen, double* dHPFilter, int iHPFilterKlen, \
    int iLevel, double* dInput, int iInputLen, double** dOutput);

// 입력된 정수 iInput이 2의 몇 제곱인지를 계산하여 iLevel에 기록
int GetDyadicLevel(int iInput, int* iLevel);

// 열의 크기가 iInputColLen인 iLevel 레벨 DWTBank()의 결과값인 2차원 데이터 dInput을 입력으로 받아서
// 각 컬럼별 최고 확률 주기 계산 결과를 dOutput에 기록
int GetAcceleration(double** dInput, int iInputColLen, int iLevel, double* dOutput);

// 크기 n인 double 배열 vals를 컬럼이름 field_name 항목의 bson 데이터 parent에 추가
static bool append_double_array(bson_t* parent, const char* field_name, \
    const double* vals, size_t n);

// 크기 n인 string 배열 vals를 컬럼이름 field_name 항목의 bson 데이터 parent에 추가
static bool append_string_array(bson_t* parent, const char* field_name, \
    const char** vals, size_t n);

// bson 데이터 doc의 컬럼이름 field_name의 배열을 double 배열 val에 기록하고, 크기는 count에 기록
void print_double_array(const bson_t* doc, const char* field_name, double* val, int* count);

// bson 데이터 doc의 컬럼이름 field_name의 배열을 string 배열 val에 기록하고, 크기는 count에 기록
void print_string_array(const bson_t* doc, const char* field_name, char val[][NAME_SIZE], int* count);

// bson 데이터 doc의 컬럼이름 field_name의 배열을 크기는 count에 기록하고, 동적으로 할당된 2차원 string 배열을 리턴
char** read_string_array_dynamic(const bson_t* doc, const char* field_name, int* count);

// *pmgcolThisCollection 컬렉션에서 string인 "PromotionID" 컬럼이 *caPromoID와 같고, string 배열 컬럼인 "ArticleIDs"가
// 크기 iArtIDsNum 인 **ArtIDs와 같고, string 배열 컬럼인 "QueryKeywords"가 크기 iQueryKeywordsNum인 **QueryKeywords와 같은
// 도큐먼트들 가운데 가장 최근에 만들어진 것을 찾아서 double 배열 "ArticleEmbedding"을 *dArticleEmb에 저장하고,
// double 배열 "QueryEmbedding"을 *dQueryEmb에 저장
int GetEmbsByArtIDsQueryKeywords(mongoc_collection_t* pmgcolThisCollection, \
    char (*ArtIDs)[MAX_LENGTH], int iArtIDsNum, char (*QKWDs)[MAX_LENGTH], int iQueryKeywordsNum, double* dArticleEmb, double* dQueryEmb);

// 연, 월, 일, 시, 분, 초를 정수로 입력 받아서 time_t 형식의 시간 데이터로 리턴
time_t MakeLocalTime_t(int YYYY, int MM, int DD, int hh, int mi, int ss);

// DatetimeStr(Example : 2023-10-08 04:02:46)을 입력 받아서 time_t 형식의 시간 데이터로 리턴
time_t DatetimeStrToLocalTime(char* caDateTimeStr);   //For MySQL Datetime Obj

// 입력값을 빅데이터 서버의 Promotions 컬렉션에 삽입
int InsertDocuInPromotionsCollection(mongoc_collection_t* pmgcolThisCollection, \
    char* RegisterID, char* ChangerID, bool InUse, char* UpdatedDate, \
    char* PromotionID, char* RegisteredDate, char* StartDate, char* EndDate, \
    char (*ArtIDs)[MAX_LENGTH], int iArtIDsNum, char (*QKWDs)[MAX_LENGTH], int iQueryKeywordsNum, \
    char* ClassAxis, char* WriterAxis, char* Visitings, \
    double* Period, int iPeriodSize, double* Acceleration, int iAccelerationSize, \
    double* NormalClass, int iNormalClassSize, double* NormalWriter, int iNormalWriterSize, \
    double* ArticleAccumEmbedding, int iArticleAccumEmbeddingSize, double* QueryAccumEmbedding, int iQueryAccumEmbeddingSize);

// pmgcolThisCollection 컬렉션의 모든 도큐먼트 갯수를 plTotalRecordNum에 저장
int GetTotalPromoDocuNum(mongoc_collection_t* pmgcolThisCollection, long* plTotalRecordNum);

// pmgcolThisCollection 컬렉션에서 PromotionID == caPromoID인 도큐먼트 갯수를 plTotalRecordNum에 저장
int GetPromoDocuNumByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, long* plTotalRecordNum);

// MongoDB ISODate(int64_t, milliseconds since epoch) → "YYYY-MM-DD HH:MM:SS" 문자열로 변환
void MongoISODateToString(int64_t mongo_time_ms, char* buffer, size_t buf_size);

// ObjectId → time_t 변환 함수
time_t GetTimeFromObjectId(const bson_oid_t* oid);

// ObjectId 문자열에서 날짜를 추출하는 함수
void PrintDateFromObjectIdString(const char* oid_str, char* buffer, int buf_size);

// 도큐먼트에서 _id 추출 후 문자열로 변환하는 함수
void GetObjectIdFromDocument(const bson_t* doc, char* out_oid_str, size_t buf_size);

// pmgcolThisCollection 컬렉션에서 PromotionID == caPromoID인 도큐먼트 lTotalRecordNum개를 받아서 pRPDR 구조체 배열의 내용을 기록
int GetRPDRByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, RegiPromoDlgRec* pRPDR, long lTotalRecordNum);

// pmgcolThisCollection 컬렉션에서 컬럼 PromotionID == caPromoID, 컬럼 _id == caInsertedDate인 도큐먼트의 컬럼 ChangerID = caChangerID, 
// 컬럼 InUse = false, 컬럼 UpdatedDate = now로 업데이트 한다.
int UpdateDocuByPromoIDArtIDsQueryKeywordsVisitings(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, \
    char* caInsertedDate, char* caChangerID);

// 문자열 복사 (strdup 대체용), add_part()에서 호출됨
char* my_strdup(char* s);

// 문자열 배열에 추가, split_url_all()에서 호출됨
void add_part(char*** parts, int* size, int* capacity, char* token);

// URL 분리 함수 (/, ., ?, =, & 모두 구분자로 사용)
char** split_url_all(char* url, int* count);

// 입력받은 str문자열에서 숫자가 아닌 모든 문자들을 제거
void keep_digits(char* str);

// pmgcolThisCollection 컬렉션에서 ArticleID == caArtID인 도큐먼트를 찾아서 *pArticle에 저장
int GetArticleDocuByArticleID(mongoc_collection_t* pmgcolThisCollection, char* caArtID, ArticleDocu* pArticle);

// Visitors 컬렉션에서 "cid", "email", "name", "Phone Number", "phone", "MemberID", "Recommender name" 등의 
// 7개 컬럼들 가운데 적어도 하나만 일치해도 되는 조건으로 도큐먼트를 찾아서 그 개수를 *iVisitorDocuNum에 기록
int GetVisitorDocuNum(mongoc_collection_t* pmgcolThisCollection, char* caCID, char* caName, char* caPhone, \
    char* caMail, char* caID, char* caRecommenders, int* iVisitorDocuNum);

// Visitors 컬렉션에서 "cid", "email", "name", "Phone Number", "phone", "MemberID", "Recommender name" 등의 
// 7개 컬럼들 가운데 적어도 하나만 일치해도 되는 조건으로 도큐먼트를 찾아서 pVisitors[]에 기록
int GetVisitorDocu(mongoc_collection_t* pmgcolThisCollection, char* caCID, char* caName, char* caPhone, \
    char* caMail, char* caID, char* caRecommenders, VisitorDocu* pVisitors);

// pmgcolThisCollection 컬렉션에서 cid == caCID인 도큐먼트를 가져와서 *pAnal에 기록
int GetAnalDocuByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, AnalDocu* pAnal);

