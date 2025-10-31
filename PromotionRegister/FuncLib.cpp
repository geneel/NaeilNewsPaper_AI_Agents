#include "pch.h"
#include "FuncLib.h"


int CStringCSVToMultiArray(CString csData, char words[][MAX_LENGTH], int* rslt_count)
{
    // CString → char 배열 변환
    CT2CA pszConvertedAnsiString(csData);
    const char* szStr = pszConvertedAnsiString;

    // 원본 복사용 버퍼
    char* buffer = (char*)malloc(STR_LENGTH);
    if (!buffer) return -1;

    // 안전 복사 (_TRUNCATE: 너무 길면 잘라냄)
    strncpy_s(buffer, STR_LENGTH, szStr, _TRUNCATE);
    buffer[STR_LENGTH - 1] = '\0';

    char* context = NULL;
    int count = 0;

    // 토큰 분리
    char* token = strtok_s(buffer, ",", &context);
    while (token != NULL && count < MAX_TOKENS)
    {
        strncpy_s(words[count], MAX_LENGTH, token, _TRUNCATE);
        words[count][MAX_LENGTH - 1] = '\0';
        count++;
        token = strtok_s(NULL, ",", &context);
    }

    *rslt_count = count;
    free(buffer);

    return 0;
}

// 입력 문자열에서 모든 공백 제거
void RemoveSpaces(char* str)
{
    char* p1 = str;  // 읽기 포인터
    char* p2 = str;  // 쓰기 포인터

    while (*p1 != '\0')
    {
        if (*p1 != ' ')  // 스페이스가 아니면 복사
        {
            *p2 = *p1;
            p2++;
        }
        p1++;
    }
    *p2 = '\0'; // 마지막에 널 종료
}

// 문자열을 "키:값" 형식에서 Dictionary 구조체로 파싱
int ParseKeyValue(const char* input, Dictionary* dict)
{
    if (!input || !dict) return -1;

    // ':' 위치 찾기
    const char* colon = strchr(input, ':');
    if (!colon) {
        return -2; // 콜론 없음 → 잘못된 입력
    }

    // Key 복사
    size_t keyLen = colon - input;
    if (keyLen >= MAX_LENGTH) keyLen = MAX_LENGTH - 1;
    strncpy_s(dict->Key, MAX_LENGTH, input, keyLen);
    dict->Key[MAX_LENGTH - 1] = '\0';
    dict->Key[keyLen] = '\0';

    // Value 복사
    const char* valueStart = colon + 1;
    strncpy_s(dict->Value, MAX_LENGTH, valueStart, MAX_LENGTH - 1);
    dict->Value[MAX_LENGTH - 1] = '\0';

    return 0; // 성공
}

int GetRecordNumByIdPwd(mongoc_collection_t* pmgcolThisCollection, char* caID, char* caPWD, int* piRecordNum)
{
    bson_t* pThisQuery;
    bson_error_t error;

    //    count = mongoc_collection_count_documents (pmgcolThisCollection, NULL, NULL, NULL, NULL, &error);
    std::string utf8_caID = CharArrayToUTF8(caID);
    std::string utf8_caPWD = CharArrayToUTF8(caPWD);
    pThisQuery = BCON_NEW("ID", BCON_UTF8(utf8_caID.c_str()), "PassWord", BCON_UTF8(utf8_caPWD.c_str()));
    *piRecordNum = (int)mongoc_collection_count_documents(pmgcolThisCollection, pThisQuery, NULL, NULL, NULL, &error);
    bson_destroy(pThisQuery);
    if (*piRecordNum < 0)
    {
        printf("%s\n", error.message);
        AfxMessageBox(_T("FuncLib.cpp-GetRecordNumByIdPwd()"));
        fprintf(stderr, "%s\n", error.message);
    }

    return 0;
}

// 간단히 SHA256으로 해시 (Windows CNG API 사용)
std::string HashPassword(const std::string& password) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status;
    DWORD hashObjectSize = 0, dataLen = 0;
    PBYTE hashObject = nullptr;
    BYTE hash[32]; // SHA256 = 32 bytes
    DWORD hashLen = sizeof(hash);

    // SHA256 알고리즘 열기
    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status < 0) throw std::runtime_error("BCryptOpenAlgorithmProvider failed");

    // 해시 객체 크기 가져오기
    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjectSize, sizeof(DWORD), &dataLen, 0);
    if (status < 0) throw std::runtime_error("BCryptGetProperty failed");

    hashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, hashObjectSize);

    // 해시 핸들 생성
    status = BCryptCreateHash(hAlg, &hHash, hashObject, hashObjectSize, nullptr, 0, 0);
    if (status < 0) throw std::runtime_error("BCryptCreateHash failed");

    // 데이터 해시
    status = BCryptHashData(hHash, (PUCHAR)password.data(), (ULONG)password.size(), 0);
    if (status < 0) throw std::runtime_error("BCryptHashData failed");

    // 최종 해시값 가져오기
    status = BCryptFinishHash(hHash, hash, hashLen, 0);
    if (status < 0) throw std::runtime_error("BCryptFinishHash failed");

    // Hex 문자열로 변환
    std::string hashHex;
    char buf[3];
    for (DWORD i = 0; i < hashLen; i++) {
        sprintf_s(buf, "%02x", hash[i]);
        hashHex += buf;
    }

    if (hHash) BCryptDestroyHash(hHash);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    if (hashObject) HeapFree(GetProcessHeap(), 0, hashObject);

    return hashHex;
}

std::string CStringToStdString(const CString& cstr)
{
    // CString → const char*
    CT2CA pszConvertedAnsiString(cstr);
    // const char* → std::string
    return std::string(pszConvertedAnsiString);
}

std::string CStringToUtf8(const CString& cstr)
{
    // CString(=wchar_t*) → UTF-8 멀티바이트 변환
    CT2A pszUtf8(cstr, CP_UTF8);

    return std::string(pszUtf8);
}

CString Utf8ToCString(const std::string& utf8str)
{
    CA2W pszUnicode(utf8str.c_str(), CP_UTF8);
    return CString(pszUnicode);
}

// CP949 → UTF-8 변환
std::string CharArrayToUTF8(const char* src)
{
    // 1단계: MultiByte(CP949) → WideChar(UTF-16)
    int wlen = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, src, -1, &wstr[0], wlen);

    // 2단계: WideChar(UTF-16) → UTF-8
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8(u8len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8[0], u8len, NULL, NULL);

    return utf8;
}

std::string CStringObjToUTF8(const CString& src)
{
    int u8len = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
    std::string utf8(u8len, 0);
    WideCharToMultiByte(CP_UTF8, 0, src, -1, &utf8[0], u8len, NULL, NULL);
    return utf8;
}

void ConvertStringArrayToCharArray(char (*dest)[MAX_LENGTH], const std::string* src, int count)
{
    for (int i = 0; i < count; i++) {
#ifdef _MSC_VER
        strcpy_s(dest[i], MAX_LENGTH, src[i].c_str());
#else
        strncpy(dest[i], src[i].c_str(), MAX_LENGTH - 1);
        dest[i][63] = '\0';
#endif
    }
}

// UTF-8 → Windows ANSI (CP_ACP) 변환 함수
// in_utf8: 입력 UTF-8 문자열
// out_ansi: 변환 결과를 받을 버퍼
// out_size: 버퍼 크기 (바이트 단위)
void Utf8ToAnsi(const char* in_utf8, char* out_ansi, size_t out_size)
{
    if (in_utf8 == NULL || out_ansi == NULL || out_size == 0) {
        return;
    }

    // UTF-8 → UTF-16 변환
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, in_utf8, -1, NULL, 0);
    if (wide_len <= 0) {
        out_ansi[0] = '\0';
        return;
    }

    WCHAR* wide_buf = (WCHAR*)malloc(wide_len * sizeof(WCHAR));
    if (!wide_buf) {
        out_ansi[0] = '\0';
        return;
    }

    MultiByteToWideChar(CP_UTF8, 0, in_utf8, -1, wide_buf, wide_len);

    // UTF-16 → ANSI 변환
    int ansi_len = WideCharToMultiByte(CP_ACP, 0, wide_buf, -1, out_ansi, (int)out_size, NULL, NULL);

    if (ansi_len == 0) {
        // 변환 실패 시 빈 문자열 반환
        out_ansi[0] = '\0';
    }

    free(wide_buf);
}

// ---------------------------------------------------------------------------
// 내부 함수: 문자열이 이미 UTF-8 형식인지 검사
// ---------------------------------------------------------------------------
inline bool IsValidUTF8_Win(const char* str)
{
    if (!str) return false;
    const unsigned char* bytes = (const unsigned char*)str;
    while (*bytes)
    {
        if ((*bytes & 0x80) == 0x00) { // 1바이트 ASCII
            bytes++;
            continue;
        }

        int bytesToCheck = 0;
        if ((*bytes & 0xE0) == 0xC0) bytesToCheck = 1;
        else if ((*bytes & 0xF0) == 0xE0) bytesToCheck = 2;
        else if ((*bytes & 0xF8) == 0xF0) bytesToCheck = 3;
        else return false;

        for (int i = 1; i <= bytesToCheck; i++)
        {
            if ((bytes[i] & 0xC0) != 0x80) return false;
        }
        bytes += bytesToCheck + 1;
    }
    return true;
}

// ---------------------------------------------------------------------------
// ANSI (CP949 등) → UTF-8 변환
// ---------------------------------------------------------------------------
inline std::string CharArrayToUTF8_Win(const char* ansiStr)
{
    if (ansiStr == nullptr || ansiStr[0] == '\0')
        return std::string("");

    // 이미 UTF-8이면 그대로 반환
    if (IsValidUTF8(ansiStr))
        return std::string(ansiStr);

    // (1) ANSI → UTF-16
    int wlen = MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, NULL, 0);
    if (wlen <= 0) return std::string("");

    std::wstring wbuf(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, &wbuf[0], wlen);

    // (2) UTF-16 → UTF-8
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8len <= 0) return std::string("");

    std::string u8buf(u8len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), -1, &u8buf[0], u8len, NULL, NULL);

    return u8buf;
}

// ---------------------------------------------------------------------------
// UTF-8 → ANSI (CP949 등) 변환
// ---------------------------------------------------------------------------
inline std::string UTF8ToCharArray_Win(const char* utf8Str)
{
    if (utf8Str == nullptr || utf8Str[0] == '\0')
        return std::string("");

    // UTF-8 유효성 검사
    if (!IsValidUTF8(utf8Str))
        return std::string(utf8Str); // 이미 ANSI라면 그대로 반환

    // (1) UTF-8 → UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
    if (wlen <= 0) return std::string("");

    std::wstring wbuf(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wbuf[0], wlen);

    // (2) UTF-16 → ANSI
    int alen = WideCharToMultiByte(CP_ACP, 0, wbuf.c_str(), -1, NULL, 0, NULL, NULL);
    if (alen <= 0) return std::string("");

    std::string abuf(alen, 0);
    WideCharToMultiByte(CP_ACP, 0, wbuf.c_str(), -1, &abuf[0], alen, NULL, NULL);

    return abuf;
}

// ---------------------------------------------------------------------------
// 1️⃣ UTF-8 유효성 검사
// ---------------------------------------------------------------------------
bool IsValidUTF8(const char* str)
{
    if (!str) return false;
    const unsigned char* bytes = (const unsigned char*)str;
    while (*bytes)
    {
        if ((*bytes & 0x80) == 0x00) { // ASCII
            bytes++;
            continue;
        }

        int extra = 0;
        if ((*bytes & 0xE0) == 0xC0) extra = 1;       // 2바이트
        else if ((*bytes & 0xF0) == 0xE0) extra = 2;  // 3바이트
        else if ((*bytes & 0xF8) == 0xF0) extra = 3;  // 4바이트
        else return false;

        for (int i = 1; i <= extra; i++)
        {
            if ((bytes[i] & 0xC0) != 0x80) return false;
        }
        bytes += extra + 1;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 2️⃣ ANSI → UTF-8 변환 (CP949 → UTF-8)
//     dst_size는 바이트 단위이며, 항상 '\0' 보장됨
// ---------------------------------------------------------------------------
bool CharArrayToUTF8(const char* ansiStr, char* utf8Buf, size_t dst_size)
{
    if (!ansiStr || !utf8Buf || dst_size == 0)
        return false;

    utf8Buf[0] = '\0';

    // 입력이 이미 UTF-8이면 그대로 복사
    if (IsValidUTF8(ansiStr))
    {
        strncpy_s(utf8Buf, dst_size, ansiStr, _TRUNCATE);
        utf8Buf[dst_size - 1] = '\0';
        return true;
    }

    // (1) ANSI → UTF-16
    int wlen = MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, NULL, 0);
    if (wlen <= 0) return false;

    WCHAR* wbuf = (WCHAR*)_alloca(wlen * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, wbuf, wlen);

    // (2) UTF-16 → UTF-8
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
    if (u8len <= 0) return false;

    char* temp = (char*)_alloca(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, temp, u8len, NULL, NULL);

    strncpy_s(utf8Buf, dst_size, temp, _TRUNCATE);
    utf8Buf[dst_size - 1] = '\0';
    return true;
}

// ---------------------------------------------------------------------------
// 3️⃣ UTF-8 → ANSI 변환 (UTF-8 → CP949)
// ---------------------------------------------------------------------------
bool UTF8ToCharArray(const char* utf8Str, char* ansiBuf, size_t dst_size)
{
    if (!utf8Str || !ansiBuf || dst_size == 0)
        return false;

    ansiBuf[0] = '\0';

    // 이미 ANSI라면 그대로 복사
    if (!IsValidUTF8(utf8Str))
    {
        strncpy_s(ansiBuf, dst_size, utf8Str, _TRUNCATE);
        ansiBuf[dst_size - 1] = '\0';
        return true;
    }

    // (1) UTF-8 → UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
    if (wlen <= 0) return false;

    WCHAR* wbuf = (WCHAR*)_alloca(wlen * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wbuf, wlen);

    // (2) UTF-16 → ANSI
    int alen = WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL);
    if (alen <= 0) return false;

    char* temp = (char*)_alloca(alen);
    WideCharToMultiByte(CP_ACP, 0, wbuf, -1, temp, alen, NULL, NULL);

    strncpy_s(ansiBuf, dst_size, temp, _TRUNCATE);
    ansiBuf[dst_size - 1] = '\0';
    return true;
}

int GetRecNum(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, char* caToken, char* caID, \
    int* ipTotalNum)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char caSQL[STR_SIZE];

    strncpy_s(caSQL, STR_SIZE, "SELECT COUNT(*) FROM ", _TRUNCATE);
    caSQL[STR_SIZE - 1] = '\0';
    strncat_s(caSQL, STR_SIZE, TB_NAME, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, " WHERE ", _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, caToken, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, " = \'", _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, caID, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, "\'", _TRUNCATE);
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
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
        strncpy_s(caError, READ_BUF_SIZE, mysql_error(DB_conn), _TRUNCATE);
        caError[READ_BUF_SIZE - 1] = '\0';
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);
        AfxMessageBox(_T("FuncLib.cpp-GetRecNum()"));

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }

    return 0;
}

int GetDatesByPromoID(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, char* caToken, char* caID, \
    char* caRegDate, int iRegDSize, char* caStartDate, int iStartDSize, char* caEndDate, int iEndDSize)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char caSQL[STR_SIZE];

    strncpy_s(caSQL, STR_SIZE, "SELECT REG_DT, PERIOD_START_DT, PERIOD_END_DT FROM ", _TRUNCATE);
    caSQL[STR_SIZE - 1] = '\0';
    strncat_s(caSQL, STR_SIZE, TB_NAME, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, " WHERE ", _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, caToken, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, " = \'", _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, caID, _TRUNCATE);
    strncat_s(caSQL, STR_SIZE, "\'", _TRUNCATE);
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
            {
                strncpy_s(caRegDate, iRegDSize, DB_row[0], _TRUNCATE);
                caRegDate[iRegDSize - 1] = '\0';
            }
            else
            {
                caRegDate[0] = '\0';
            }
            if (DB_row[1])
            {
                strncpy_s(caStartDate, iStartDSize, DB_row[1], _TRUNCATE);
                caStartDate[iStartDSize - 1] = '\0';
            }
            else
            {
                caStartDate[0] = '\0';
            }
            if (DB_row[2])
            {
                strncpy_s(caEndDate, iEndDSize, DB_row[2], _TRUNCATE);
                caEndDate[iEndDSize - 1] = '\0';
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
        strncpy_s(caError, READ_BUF_SIZE, mysql_error(DB_conn), _TRUNCATE);
        caError[READ_BUF_SIZE - 1] = '\0';
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);
        AfxMessageBox(_T("FuncLib.cpp-GetDatesByPromoID()"));

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }

    return 0;
}

int GetTotalRecNum(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, char* TB_NAME, \
    long* ipTotalNum)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char caSQL[STR_SIZE];

    strncpy_s(caSQL, STR_SIZE, "SELECT COUNT(*) FROM ", _TRUNCATE);
    caSQL[STR_SIZE - 1] = '\0';
    strncat_s(caSQL, STR_SIZE, TB_NAME, _TRUNCATE);
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
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
        strncpy_s(caError, READ_BUF_SIZE, mysql_error(DB_conn), _TRUNCATE);
        caError[READ_BUF_SIZE - 1] = '\0';
        printf("\nErrror Number: %d,    Error: %s\n", iErrno, caError);
        AfxMessageBox(_T("FuncLib.cpp-GetTotalRecNum()"));

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }

    return 0;
}

int GetAxis(MYSQL* DB_connection, MYSQL* DB_conn, char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME,
    char* COL_NAME, char* TB_NAME,
    int iTotalNum, char** caaAxis, int iStrSize)
{
    if (!DB_conn || !caaAxis || iTotalNum <= 0 || iStrSize <= 0) return -1;

    char caSQL[STR_SIZE];
    int n = snprintf(caSQL, sizeof(caSQL), "SELECT %s FROM %s",
                     COL_NAME ? COL_NAME : "", TB_NAME ? TB_NAME : "");
    if (n < 0 || n >= (int)sizeof(caSQL)) return -2;

    if (mysql_query(DB_conn, caSQL) != 0) {
        int iErrno = mysql_errno(DB_conn);
        printf("\nError Number: %d, Error: %s\n", iErrno, mysql_error(DB_conn));
        AfxMessageBox(_T("FuncLib.cpp-GetAxis()"));
        if (iErrno == 2013) {
            mysql_init(DB_connection);
            // ⚠️ 포인터 갱신 필요시 MYSQL** 로 바꾸세요
            MYSQL* newc = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, nullptr, 0);
            // DB_conn = newc; // 값전달이라 의미 없음
        }
        return -3;
    }

    MYSQL_RES* DB_result = mysql_store_result(DB_conn);
    if (!DB_result) return -4;

    int i = 0;
    MYSQL_ROW DB_row;
    while (i < iTotalNum && (DB_row = mysql_fetch_row(DB_result)) != nullptr) {
        // 방어: 포인터/버퍼 체크
        if (!caaAxis[i]) { i++; continue; }

        if (DB_row[0]) {
            // 문자열 복사 (항상 널 종료)
            strncpy_s(caaAxis[i], iStrSize, DB_row[0], _TRUNCATE);
            caaAxis[i][iStrSize - 1] = '\0';
        } else {
            caaAxis[i][0] = '\0';
        }
        i++;
    }
    mysql_free_result(DB_result);

    // 남은 슬롯들을 빈 문자열로 초기화(디버그/후처리 안전)
    for (int j = i; j < iTotalNum; ++j) {
        if (caaAxis[j]) caaAxis[j][0] = '\0';
    }

#ifdef MANUAL_DEBUG
    FILE* fp = nullptr;
    if (fopen_s(&fp, "Out1.txt", "w") == 0 && fp) {
        for (int j = 0; j < iTotalNum; j++) {
            if (caaAxis[j] && caaAxis[j][0] != '\0') {
                fprintf(fp, "caaAxis[%d] : %s\n", j, caaAxis[j]);
            } else {
                fprintf(fp, "caaAxis[%d] : (null or empty)\n", j);
            }
        }
        fclose(fp);
    }
#endif
    return 0;
}

int GetClassIdxWriterIdxFromArtID(MYSQL* DB_connection, MYSQL* DB_conn, \
    char* DB_HOST, char* DB_USER, char* DB_PASS, char* DB_NAME, \
    char** caaClassAxis, char** caaWriterAxis, \
    int iTotalClassNum, int iTotalWriterNum, char* ART_ID, \
    int* iClassIdx, int* iWriterIdx)
{
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char caSQL[STR_SIZE], caTmp[READ_BUF_SIZE] = { 0 };

    strncpy_s(caSQL, STR_SIZE, "SELECT CODE_ID FROM naeil_wms_db.WMS_ARTICLE_CODE WHERE ART_ID = ", _TRUNCATE);
    caSQL[STR_SIZE - 1] = '\0';
    strncat_s(caSQL, STR_SIZE, ART_ID, _TRUNCATE);
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
            {
                strncpy_s(caTmp, READ_BUF_SIZE, DB_row[0], _TRUNCATE);
                caTmp[READ_BUF_SIZE - 1] = '\0';
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
        strncpy_s(caError, READ_BUF_SIZE, mysql_error(DB_conn), _TRUNCATE);
        caError[READ_BUF_SIZE - 1] = '\0';

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }
    caSQL[0] = '\0';

    bool bClassFault = true;
    for (int i = 0; i < iTotalClassNum; i++)
    {
        if (!strcmp(caaClassAxis[i], caTmp))
        {
            *iClassIdx = i;
            bClassFault = false;
            break;
        }
    }
    if (bClassFault) *iClassIdx = -1;
    caTmp[0] = '\0';

    strncpy_s(caSQL, STR_SIZE, "SELECT USER_ID FROM naeil_wms_db.WMS_ARTICLE_WRITER WHERE ART_ID = ", _TRUNCATE);
    caSQL[STR_SIZE - 1] = '\0';
    strncat_s(caSQL, STR_SIZE, ART_ID, _TRUNCATE);
    if (mysql_query(DB_conn, caSQL) == 0)
    {
        DB_result = mysql_store_result(DB_conn);
        while ((DB_row = mysql_fetch_row(DB_result)) != NULL) {
            if (DB_row[0])
            {
                strncpy_s(caTmp, READ_BUF_SIZE, DB_row[0], _TRUNCATE);
                caTmp[READ_BUF_SIZE - 1] = '\0';
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
        strncpy_s(caError, READ_BUF_SIZE, mysql_error(DB_conn), _TRUNCATE);
        caError[READ_BUF_SIZE - 1] = '\0';

        if (iErrno == 2013)
        {
            mysql_init(DB_connection);
            DB_conn = mysql_real_connect(DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char*)NULL, 0);
        }
    }
    caSQL[0] = '\0';

    bool bWriterFault = true;
    for (int i = 0; i < iTotalWriterNum; i++)
    {
        if (!strcmp(caaWriterAxis[i], caTmp))
        {
            *iWriterIdx = i;
            bWriterFault = false;
            break;
        }
    }
    if (bWriterFault) *iWriterIdx = -1;
    caTmp[0] = '\0';

    return 0;
}

int Normalize(double* dNormal, int iTotalNum)
{
    double dMax = dNormal[0];

    for (int i = 0; i < iTotalNum; i++)
    {
        if (dMax < dNormal[i]) dMax = dNormal[i];
    }
    if (dMax != 0)
    {
        for (int i = 0; i < iTotalNum; i++)
        {
            dNormal[i] = dNormal[i] / dMax;
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
void gaussian_smooth_1d(const double* in, double* out, int n, double sigma) {
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
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
        if (j > i) {
            SWAP(data[j], data[i]);
            SWAP(data[j + 1], data[i + 1]);
        }
        m = nn;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax = 2;
    while (n > mmax) {
        istep = mmax << 1;
        theta = isign * (6.28318530717959 / mmax);
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                j = i + mmax;
                tempr = wr * data[j] - wi * data[j + 1];
                tempi = wr * data[j + 1] + wi * data[j];
                data[j] = data[i] - tempr;
                data[j + 1] = data[i + 1] - tempi;
                data[i] += tempr;
                data[i + 1] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}

//  배열 data[]의 시작 인덱스가 0이 아닌 1이라는 사실에 유의
//  따라서 data[]의 크기는 (nn*2 + 1)이다.
//  반면에 dRslt[]의 크기는 nn이며 시작 인덱스는 0이다.
int PowerSpectralDensity(double data[], int nn, double* dRslt)
{
    for (int i = 0; i < nn; i++)
    {
        dRslt[i] = sqrt(pow(data[i * 2 + 1], 2) + pow(data[i * 2 + 2], 2));
    }

    return 0;
}

int DWT(double* dFilter, int iFilterKlen, int iLevel, double* dInput, int iInputLen, double* dOutput)
{
    if (iLevel <= 0) iLevel = 1;

    int iFiltLen = (iFilterKlen - 1) * pow(2, iLevel - 1) + 1;
    double* dFilt = (double*)malloc(sizeof(double) * iFiltLen);
    for (int i = 0; i < iFilterKlen; i++)
    {
        for (int j = (i * pow(2, iLevel - 1)); j < ((i * pow(2, iLevel - 1)) + pow(2, iLevel - 1)); j++)
        {
            if (j == (i * pow(2, iLevel - 1)))
            {
                dFilt[j] = dFilter[i];
                if (i == (iFilterKlen - 1)) break;
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

    for (int i = 0; i < iInputLen; i++)
    {
        dOutput[0][i] = dInput[i];
    }

    for (int i = 0; i < iLevel - 1; i++)
    {
        DWT(dLPFilter, iLPFilterKlen, i + 1, dOutput[i], iInputLen, dOutput[i + 1]);
    }

    return 0;
}

int GetDyadicLevel(int iInput, int* iLevel)
{
    int iTmp, iQ, iR;
    iTmp = iInput;
    *iLevel = 0;
    do
    {
        iQ = iTmp / 2;
        iR = iTmp % 2;
        iTmp = iQ;

        if (iR != 0) break;
        (*iLevel)++;
    } while (true);

    return 0;
}

int GetAcceleration(double** dInput, int iInputColLen, int iLevel, double* dOutput)
{
    int* iIdx = (int*)malloc(sizeof(int) * iInputColLen);

    for (int i = 0; i < iInputColLen; i++)
    {
        double dMax = dInput[0][i];
        iIdx[i] = 0;
        for (int j = 0; j < iLevel; j++)
        {
            if (dInput[j][i] > dMax)
            {
                dMax = dInput[j][i];
                iIdx[i] = j;
            }
#ifdef MANUAL_DEBUG
            printf("\nIn GetAcceleration() : dInput[%d][%d] = %lf, ", i, j, dInput[i][j]);
#endif
        }
    }
    for (int i = 0; i < iInputColLen; i++)
    {
        dOutput[i] = pow(2, iIdx[i]);
    }

    free(iIdx);

    return 0;
}

static bool append_double_array(bson_t* parent, const char* field_name, \
    const double* vals, size_t n)
{
    bson_t arr;
    char key_buf[16];
    const char* keyptr;            // ← 여기!
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

static bool append_string_array(bson_t* parent, const char* field_name, \
    char (*vals)[MAX_LENGTH], size_t n)
{
    bson_t arr;
    char key_buf[16];
    const char* keyptr;

    if (!bson_append_array_begin(parent, field_name, -1, &arr)) {
        return false;
    }

    for (uint32_t i = 0; i < (uint32_t)n; i++) {
        // 배열 인덱스를 문자열 키("0", "1", ...)로 변환
        bson_uint32_to_string(i, &keyptr, key_buf, sizeof key_buf);

        // 문자열 원소 추가
        if (!bson_append_utf8(&arr, keyptr, -1, vals[i], -1)) {
            bson_append_array_end(parent, &arr);
            return false;
        }
    }

    if (!bson_append_array_end(parent, &arr)) {
        return false;
    }
    return true;
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
                    strncpy_s(val[*count], NAME_SIZE, str, _TRUNCATE);
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

char** read_string_array_dynamic(const bson_t* doc, const char* field_name, int* count)
{
    *count = 0;
    bson_iter_t iter;

    // 필드 존재 및 타입 확인
    if (!bson_iter_init_find(&iter, doc, field_name) || !BSON_ITER_HOLDS_ARRAY(&iter)) {
        printf("Field \"%s\" not found or not an array.\n", field_name);
        return NULL;
    }

    const uint8_t* array_buf;
    uint32_t array_len;
    bson_t array;

    bson_iter_array(&iter, &array_len, &array_buf);
    bson_init_static(&array, array_buf, array_len);

    bson_iter_t arr_iter;
    if (!bson_iter_init(&arr_iter, &array)) {
        return NULL;
    }

    // 1차 스캔: 요소 개수 세기
    int temp_count = 0;
    while (bson_iter_next(&arr_iter)) {
        if (BSON_ITER_HOLDS_UTF8(&arr_iter)) {
            temp_count++;
        }
    }

    if (temp_count == 0) {
        return NULL;
    }

    // 2차 스캔: 문자열 복사
    char** result = (char**)malloc(sizeof(char*) * temp_count);
    if (!result) return NULL;

    bson_iter_init(&arr_iter, &array);
    int idx = 0;
    while (bson_iter_next(&arr_iter)) {
        if (BSON_ITER_HOLDS_UTF8(&arr_iter)) {
            const char* str = bson_iter_utf8(&arr_iter, NULL);
            result[idx] = (char*)malloc(strlen(str) + 1);
            strncpy_s(result[idx], strlen(str) + 1, str, _TRUNCATE);
            result[idx][strlen(str)] = '\0';
            idx++;
        }
    }

    *count = temp_count;
    return result;
}

int GetEmbsByArtIDsQueryKeywords(mongoc_collection_t* pmgcolThisCollection, \
    char (*ArtIDs)[MAX_LENGTH], int iArtIDsNum, char (*QKWDs)[MAX_LENGTH], int iQueryKeywordsNum, double* dArticleEmb, double* dQueryEmb)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    bool bNotYetArticle = true;
    bool bNotYetQuery = true;


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    //std::string utf8_caPromoID = CharArrayToUTF8(caPromoID);
    //BSON_APPEND_UTF8(pThisQuery, "PromotionID", utf8_caPromoID.c_str());

    // (2) ArticleIDs 배열 조건 (정확히 동일한 배열)
    std::string utf8_ArtIDs[MIN_TOKENS];
    char caa_utf8_ArtIDs[MIN_TOKENS][MAX_LENGTH];
    for (int i = 0; i < iArtIDsNum; i++)
    {
        utf8_ArtIDs[i] = CharArrayToUTF8(ArtIDs[i]);
    }
    ConvertStringArrayToCharArray(caa_utf8_ArtIDs, utf8_ArtIDs, iArtIDsNum);
    append_string_array(pThisQuery, "ArticleIDs", caa_utf8_ArtIDs, iArtIDsNum);

    // (3) QueryKeywords 배열 조건
    std::string utf8_QKWDs[MIN_TOKENS];
    char caa_utf8_QKWDs[MIN_TOKENS][MAX_LENGTH];
    for (int i = 0; i < iQueryKeywordsNum; i++)
    {
        utf8_QKWDs[i] = CharArrayToUTF8(QKWDs[i]);
    }
    ConvertStringArrayToCharArray(caa_utf8_QKWDs, utf8_QKWDs, iQueryKeywordsNum);
    append_string_array(pThisQuery, "QueryKeywords", caa_utf8_QKWDs, iQueryKeywordsNum);

    // (4) 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    //BSON_APPEND_INT32(pThisOpts, "limit", 1);

    // (5) 쿼리 실행
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    // (6) 결과 확인
    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) 
        {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            AfxMessageBox(_T("FuncLib.cpp-GetEmbsByArtIDsQueryKeywords() : JSON이 아니거나 루트가 Object가 아님"));
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return FALSE;
        }

        //// (1) InUse 필드 검사
        //bson_iter_t iter;
        //bool bInUse = false;

        //if (bson_iter_init_find(&iter, pThisDoc, "InUse") && BSON_ITER_HOLDS_BOOL(&iter)) {
        //    bInUse = bson_iter_bool(&iter);
        //}

        //// (2) InUse == true일 때만 실행
        //if (bInUse) {
            int iDummy;
            if (bNotYetArticle)
            {
                print_double_array(pThisDoc, "ArticleEmbedding", dArticleEmb, &iDummy);
                for (int i = 0; i < iDummy; i++)
                {
                    if (dArticleEmb[i] != 0) bNotYetArticle = false;
                }
            }
            if (bNotYetQuery)
            {
                print_double_array(pThisDoc, "QueryEmbedding", dQueryEmb, &iDummy);
                for (int i = 0; i < iDummy; i++)
                {
                    if (dQueryEmb[i] != 0) bNotYetQuery = false;
                }
            }
            //}

        bson_free(pThisStr);
    }
    if (mongoc_cursor_error(pThisCursor, &error)) {
        fprintf(stderr, "Cursor error: %s\n", error.message);
        AfxMessageBox(_T("FuncLib.cpp-GetEmbsByArtIDsQueryKeywords() : Cursor error"));
        bson_destroy(pThisQuery);
        bson_destroy(pThisOpts);
        bson_destroy(&sort);
        mongoc_cursor_destroy(pThisCursor);
        return FALSE;
    }


    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    return 0;
}

time_t MakeLocalTime_t(int YYYY, int MM, int DD, int hh, int mi, int ss)
{
    struct tm st_tm;

    st_tm.tm_year = YYYY - 1900;
    st_tm.tm_mon = MM - 1;
    st_tm.tm_mday = DD;
    st_tm.tm_hour = hh;
    st_tm.tm_min = mi;
    st_tm.tm_sec = ss;

    return mktime(&st_tm);
}

time_t DatetimeStrToLocalTime(char* caDateTimeStr)   //For MySQL Datetime Obj
{   //DatetimeStr Example : 2023-10-08 04:02:46
    int i, iLength;
    int YYYY, MM, DD, hh, mi, ss;
    char caYYYY[5], caMM[3], caDD[3], cahh[3], cami[3], cass[3];

    iLength = strlen(caDateTimeStr);
    if (iLength != 19)
    {
        return MakeLocalTime_t(1, 1, 1, 1, 1, 1);
    }

    for (i = 0; i < 4; i++)
    {
        caYYYY[i] = caDateTimeStr[i];
    }
    caYYYY[4] = '\0';

    for (i = 0; i < 2; i++)
    {
        caMM[i] = caDateTimeStr[i + 5];
    }
    caMM[2] = '\0';

    for (i = 0; i < 2; i++)
    {
        caDD[i] = caDateTimeStr[i + 8];
    }
    caDD[2] = '\0';

    for (i = 0; i < 2; i++)
    {
        cahh[i] = caDateTimeStr[i + 11];
    }
    cahh[2] = '\0';

    for (i = 0; i < 2; i++)
    {
        cami[i] = caDateTimeStr[i + 14];
    }
    cami[2] = '\0';

    for (i = 0; i < 2; i++)
    {
        cass[i] = caDateTimeStr[i + 17];
    }
    cass[2] = '\0';

    return MakeLocalTime_t(atoi(caYYYY), atoi(caMM), atoi(caDD), atoi(cahh), atoi(cami), atoi(cass));
}

int InsertDocuInPromotionsCollection(mongoc_collection_t* pmgcolThisCollection, \
    char* RegisterID, char* ChangerID, bool InUse, char* UpdatedDate, \
    char* PromotionID, char* RegisteredDate, char* StartDate, char* EndDate, \
    char (*ArtIDs)[MAX_LENGTH], int iArtIDsNum, char (*QKWDs)[MAX_LENGTH], int iQueryKeywordsNum, \
    char* ClassAxis, char* WriterAxis, char* Visitings, \
    double* Period, int iPeriodSize, double* Acceleration, int iAccelerationSize, \
    double* NormalClass, int iNormalClassSize, double* NormalWriter, int iNormalWriterSize, \
    double* ArticleAccumEmbedding, int iArticleAccumEmbeddingSize, double* QueryAccumEmbedding, int iQueryAccumEmbeddingSize)
{
    bson_t* pThisInsert;
    bson_error_t ThisError;
    bool bOK = false;

    pThisInsert = bson_new();
    if (!pThisInsert) {
        fprintf(stderr, "bson_new failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : bson_new failed"));
        return FALSE;
    }

    std::string utf8_RegisterID = CharArrayToUTF8(RegisterID);
    if (!BSON_APPEND_UTF8(pThisInsert, "RegisterID", utf8_RegisterID.c_str())) {
        fprintf(stderr, "append RegisterID failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append RegisterID failed"));
        return FALSE;
    }
    std::string utf8_ChangerID = CharArrayToUTF8(ChangerID);
    if (!BSON_APPEND_UTF8(pThisInsert, "ChangerID", utf8_ChangerID.c_str())) {
        fprintf(stderr, "append ChangerID failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append ChangerID failed"));
        return FALSE;
    }
    if (InUse)
    {
        if (!BSON_APPEND_BOOL(pThisInsert, "InUse", true)) {
            fprintf(stderr, "append InUse failed in InsertDocuInPromotionsCollection()\n");
            AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append InUse failed"));
            return FALSE;
        }
    }
    else
    {
        if (!BSON_APPEND_BOOL(pThisInsert, "InUse", false)) {
            fprintf(stderr, "append InUse failed in InsertDocuInPromotionsCollection()\n");
            AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append InUse failed"));
            return FALSE;
        }
    }
    time_t tUpdatedDate;
    tUpdatedDate = DatetimeStrToLocalTime(UpdatedDate);   //For MySQL Datetime Obj    
    // MongoDB는 밀리초 단위
    long long mtUpdatedDate = (long long)tUpdatedDate * 1000;
    if (!BSON_APPEND_DATE_TIME(pThisInsert, "UpdatedDate", mtUpdatedDate)) {
        fprintf(stderr, "append UpdatedDate failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append UpdatedDate failed"));
        return FALSE;
    }

    std::string utf8_PromotionID = CharArrayToUTF8(PromotionID);
    if (!BSON_APPEND_UTF8(pThisInsert, "PromotionID", utf8_PromotionID.c_str())) {
        fprintf(stderr, "append PromotionID failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append PromotionID failed"));
        return FALSE;
    }
    time_t tRegisteredDate;
    tRegisteredDate = DatetimeStrToLocalTime(RegisteredDate);   //For MySQL Datetime Obj 
    // MongoDB는 밀리초 단위
    long long mtRegisteredDate = (long long)tRegisteredDate * 1000;
    if (!BSON_APPEND_DATE_TIME(pThisInsert, "RegisteredDate", mtRegisteredDate)) {
        fprintf(stderr, "append RegisteredDate failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append RegisteredDate failed"));
        return FALSE;
    }
    time_t tStartDate;
    tStartDate = DatetimeStrToLocalTime(StartDate);   //For MySQL Datetime Obj    
    // MongoDB는 밀리초 단위
    long long mtStartDate = (long long)tStartDate * 1000;
    if (!BSON_APPEND_DATE_TIME(pThisInsert, "StartDate", mtStartDate)) {
        fprintf(stderr, "append StartDate failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append StartDate failed"));
        return FALSE;
    }
    time_t tEndDate;
    tEndDate = DatetimeStrToLocalTime(EndDate);   //For MySQL Datetime Obj    
    // MongoDB는 밀리초 단위
    long long mtEndDate = (long long)tEndDate * 1000;
    if (!BSON_APPEND_DATE_TIME(pThisInsert, "EndDate", mtEndDate)) {
        fprintf(stderr, "append EndDate failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append EndDate failed"));
        return FALSE;
    }

    std::string utf8_ArtIDs[MIN_TOKENS];
    char caa_utf8_ArtIDs[MIN_TOKENS][MAX_LENGTH];
    for (int i = 0; i < iArtIDsNum; i++)
    {
        utf8_ArtIDs[i] = CharArrayToUTF8(ArtIDs[i]);
    }
    ConvertStringArrayToCharArray(caa_utf8_ArtIDs, utf8_ArtIDs, iArtIDsNum);
    if (!append_string_array(pThisInsert, "ArticleIDs", caa_utf8_ArtIDs, iArtIDsNum)) {
        fprintf(stderr, "append array ArticleIDs failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append ArticleIDs failed"));
        return FALSE;
    }
    std::string utf8_QKWDs[MIN_TOKENS];
    char caa_utf8_QKWDs[MIN_TOKENS][MAX_LENGTH];
    for (int i = 0; i < iQueryKeywordsNum; i++)
    {
        utf8_QKWDs[i] = CharArrayToUTF8(QKWDs[i]);
    }
    ConvertStringArrayToCharArray(caa_utf8_QKWDs, utf8_QKWDs, iQueryKeywordsNum);
    if (!append_string_array(pThisInsert, "QueryKeywords", caa_utf8_QKWDs, iQueryKeywordsNum)) {
        fprintf(stderr, "append array QueryKeywords failed in InsertDocuInArticlesCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append QueryKeywords failed"));
        return FALSE;
    }

    std::string utf8_ClassAxis = CharArrayToUTF8(ClassAxis);
    if (!BSON_APPEND_UTF8(pThisInsert, "ClassAxis", utf8_ClassAxis.c_str())) {
        fprintf(stderr, "append ClassAxis failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append ClassAxis failed"));
        return FALSE;
    }
    std::string utf8_WriterAxis = CharArrayToUTF8(WriterAxis);
    if (!BSON_APPEND_UTF8(pThisInsert, "WriterAxis", utf8_WriterAxis.c_str())) {
        fprintf(stderr, "append WriterAxis failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append WriterAxis failed"));
        return FALSE;
    }
    std::string utf8_Visitings = CharArrayToUTF8(Visitings);
    if (!BSON_APPEND_UTF8(pThisInsert, "Visitings", utf8_Visitings.c_str())) {
        fprintf(stderr, "append Visitings failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append Visitings failed"));
        return FALSE;
    }

    if (!append_double_array(pThisInsert, "Period", Period, iPeriodSize)) {
        fprintf(stderr, "append array Period failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append Period failed"));
        return FALSE;
    }
    if (!append_double_array(pThisInsert, "Acceleration", Acceleration, iAccelerationSize)) {
        fprintf(stderr, "append array Acceleration failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append Acceleration failed"));
        return FALSE;
    }
    if (!append_double_array(pThisInsert, "NormalClass", NormalClass, iNormalClassSize)) {
        fprintf(stderr, "append array NormalClass failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append NormalClass failed"));
        return FALSE;
    }
    if (!append_double_array(pThisInsert, "NormalWriter", NormalWriter, iNormalWriterSize)) {
        fprintf(stderr, "append array NormalWriter failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append NormalWriter failed"));
        return FALSE;
    }
    if (!append_double_array(pThisInsert, "ArticleEmbedding", ArticleAccumEmbedding, iArticleAccumEmbeddingSize)) {
        fprintf(stderr, "append array ArticleEmbedding failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append ArticleEmbedding failed"));
        return FALSE;
    }
    if (!append_double_array(pThisInsert, "QueryEmbedding", QueryAccumEmbedding, iQueryAccumEmbeddingSize)) {
        fprintf(stderr, "append array QueryEmbedding failed in InsertDocuInPromotionsCollection()\n");
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : append QueryEmbedding failed"));
        return FALSE;
    }

    bOK = mongoc_collection_insert_one(pmgcolThisCollection, pThisInsert, NULL, NULL, &ThisError);
    if (!bOK) {
        fprintf(stderr, "insert_one failed in InsertDocuInPromotionsCollection(): %s\n", ThisError.message);
        AfxMessageBox(_T("FuncLib.cpp-InsertDocuInPromotionsCollection() : insert_one failed"));
        return FALSE;
    }

    return 0;
}

int GetTotalPromoDocuNum(mongoc_collection_t* pmgcolThisCollection, long* plTotalRecordNum)
{
    bson_error_t error;
    bson_t* pThisFilter;

    // (1) InUse == true 조건 추가
    pThisFilter = BCON_NEW("InUse", BCON_BOOL(true));

    // (2) 문서 개수 카운트
    *plTotalRecordNum = (long)
        mongoc_collection_count_documents(
            pmgcolThisCollection,
            pThisFilter,
            NULL,   // opts
            NULL,   // read prefs
            NULL,   // reply
            &error);

    // (3) 예외 처리
    if (*plTotalRecordNum < 0)
    {
        fprintf(stderr, "Count failed: %s\n", error.message);
        AfxMessageBox(_T("FuncLib.cpp-GetTotalPromoDocuNum() : error!"));
        bson_destroy(pThisFilter);
        return FALSE;
    }

    bson_destroy(pThisFilter);
    return 0;
}

int GetPromoDocuNumByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, long* plTotalRecordNum)
{
    bson_t* pThisQuery;
    bson_error_t error;

    // UTF-8 변환
    std::string utf8_caPromoID = CharArrayToUTF8(caPromoID);

    // (1) 조건: PromotionID == caPromoID, InUse == true
    pThisQuery = BCON_NEW(
        "$and", "[",
        "{", "PromotionID", BCON_UTF8(utf8_caPromoID.c_str()), "}",
        "{", "InUse", BCON_BOOL(true), "}",
        "]"
    );

    // (2) 카운트 수행
    *plTotalRecordNum = (long)mongoc_collection_count_documents(
        pmgcolThisCollection,
        pThisQuery,
        NULL,   // opts
        NULL,   // read prefs
        NULL,   // reply
        &error);

    // (3) 메모리 해제 및 오류 처리
    bson_destroy(pThisQuery);

    if (*plTotalRecordNum < 0)
    {
        fprintf(stderr, "Count failed: %s\n", error.message);
        AfxMessageBox(_T("FuncLib.cpp-GetPromoDocuNumByPromoID() : error!"));
        return FALSE;
    }

    return 0;
}

// MongoDB ISODate(int64_t, milliseconds since epoch) → "YYYY-MM-DD HH:MM:SS" 문자열로 변환
void MongoISODateToString(int64_t mongo_time_ms, char* buffer, size_t buf_size)
{
    if (buffer == NULL || buf_size == 0)
        return;

    // (1) 밀리초 → 초로 변환
    time_t seconds = (time_t)(mongo_time_ms / 1000);

    // (2) time_t → tm 구조체 (UTC 기준)
    struct tm tm_info;
    errno_t err = gmtime_s(&tm_info, &seconds);  // UTC 시간 기준
    if (err != 0)
    {
        strncpy_s(buffer, buf_size, "1970-01-01 00:00:00", _TRUNCATE);
        buffer[buf_size - 1] = '\0';
        return;
    }

    // (3) tm 구조체 → 문자열 변환
    strftime(buffer, buf_size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

// ObjectId → time_t 변환 함수
static inline time_t GetTimeFromObjectId(const bson_oid_t* oid) {
#if BSON_CHECK_VERSION(1,17,0)
    return bson_oid_get_time_t(oid);      // libbson 1.17+
#else
    return (time_t)bson_oid_get_time(oid); // 구버전 호환
#endif
}

// ObjectId 문자열에서 날짜를 추출하는 함수
void PrintDateFromObjectIdString(const char* oid_str, char* buffer, int buf_size)
{
    if (!buffer || buf_size <= 0) return;
    buffer[0] = '\0';

    // 최소 20바이트(19 + NUL) 보장
    const char* kFallback = "1970-01-01 00:00:00";
    if (buf_size < 20) {
        // 공간이 부족하면 가능한 만큼만 안전하게 복사
        strncpy_s(buffer, buf_size, kFallback, _TRUNCATE);
        buffer[buf_size - 1] = '\0';
        printf("버퍼 크기가 너무 작습니다(>=20 필요)\n");
        return;
    }

    if (!oid_str || oid_str[0] == '\0') {
        strncpy_s(buffer, buf_size, kFallback, _TRUNCATE);
        buffer[buf_size - 1] = '\0';
        printf("ObjectId가 비어있어 기본값 사용\n");
        printf("ObjectId: (empty)\n생성일시: %s\n", buffer);
        return;
    }

    // "ObjectId('...')" 또는 "ObjectId("...")" 형태 처리
    char hex[25] = { 0 }; // 24 hex + NUL
    const char* src = oid_str;
    const char* p = NULL;
    const char* q = NULL;
    if (strncmp(src, "ObjectId(", 9) == 0) {
        // 작은따옴표 또는 큰따옴표 둘 다 허용
        p = strchr(src, '\'');
        if (!p) p = strchr(src, '"');
        if (p) {
            q = strchr(p + 1, p[0]); // 동일한 따옴표로 닫힘 찾기
            if (q && (q - (p + 1) == 24)) {
                memcpy(hex, p + 1, 24);
                hex[24] = '\0';
                src = hex;
            }
        }
    }

    // 24자리 16진 OID 검증
    if (!bson_oid_is_valid(src, 24)) {
        strncpy_s(buffer, buf_size, kFallback, _TRUNCATE);
        buffer[buf_size - 1] = '\0';
        printf("유효하지 않은 ObjectId 문자열입니다.\n");
        printf("ObjectId: %s\n생성일시: %s\n", oid_str, buffer);
        return;
    }

    // 문자열 → OID
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, src);

    // OID → 생성 시각(UTC epoch seconds)
    time_t t = GetTimeFromObjectId(&oid);

    // UTC → 로컬 시간
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) != 0) {
        strncpy_s(buffer, buf_size, kFallback, _TRUNCATE);
        buffer[buf_size - 1] = '\0';
        printf("localtime_s() 변환 실패\n");
        printf("ObjectId: %s\n생성일시: %s\n", oid_str, buffer);
        return;
    }

    // 반드시 buf_size 사용!
    size_t n = strftime(buffer, buf_size, "%Y-%m-%d %H:%M:%S", &tm_info);
    if (n == 0) { // 공간 부족 등
        buffer[buf_size - 1] = '\0';
        printf("strftime() 실패(버퍼 부족 가능)\n");
    }

    printf("ObjectId: %s\n", oid_str);
    printf("생성일시: %s\n", buffer);
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

int GetRPDRByPromoID(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, RegiPromoDlgRec *pRPDR, long lTotalRecordNum)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery;
    const bson_t* pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    long lCounter = 0;
    char caUtf8[TEXT_SIZE * 10];

    // ✅ 쿼리 생성: {"PromotionID": "caPromoID"}
    pThisQuery = bson_new();

    if (strcmp(caPromoID, ""))
    {
        std::string utf8_caPromoID = CharArrayToUTF8(caPromoID);
        BSON_APPEND_UTF8(pThisQuery, "PromotionID", utf8_caPromoID.c_str());
    }

    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, NULL, NULL);
    bson_destroy(pThisQuery);

    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            AfxMessageBox(_T("FuncLib.cpp-GetRPDRByPromoID() : JSON이 아니거나 루트가 Object가 아님"));
            return FALSE;
        }

        // (1) InUse 필드 검사
        bson_iter_t iter;
        bool bInUse = false;

        if (bson_iter_init_find(&iter, pThisDoc, "InUse") && BSON_ITER_HOLDS_BOOL(&iter)) 
        {
            bInUse = bson_iter_bool(&iter);
        }

        // (2) InUse == true일 때만 실행
        if (bInUse) 
        {
            auto PromotionID = ThisDocument.FindMember("PromotionID");
            if ((PromotionID != ThisDocument.MemberEnd()) && (PromotionID->value.IsString()))
            {
                strncpy_s(caUtf8, TEXT_SIZE * 10, PromotionID->value.GetString(), _TRUNCATE);
                caUtf8[TEXT_SIZE * 10 - 1] = '\0';
                Utf8ToAnsi(caUtf8, pRPDR[lCounter].caPromoID, READ_BUF_SIZE);
            }
            auto Visitings = ThisDocument.FindMember("Visitings");
            if ((Visitings != ThisDocument.MemberEnd()) && (Visitings->value.IsString()))
            {
                strncpy_s(caUtf8, TEXT_SIZE * 10, Visitings->value.GetString(), _TRUNCATE);
                caUtf8[TEXT_SIZE * 10 - 1] = '\0';
                Utf8ToAnsi(caUtf8, pRPDR[lCounter].caVisitorPattern, TEXT_SIZE * 10);
            }


            GetObjectIdFromDocument(pThisDoc, caUtf8, TEXT_SIZE * 10);
            PrintDateFromObjectIdString(caUtf8, pRPDR[lCounter].caInsertedDate, READ_BUF_SIZE);
            strncpy_s(caUtf8, TEXT_SIZE * 10, pRPDR[lCounter].caInsertedDate, _TRUNCATE);
            caUtf8[TEXT_SIZE * 10 - 1] = '\0';
            Utf8ToAnsi(caUtf8, pRPDR[lCounter].caInsertedDate, READ_BUF_SIZE);


            print_string_array(pThisDoc, "ArticleIDs", pRPDR[lCounter].caaArtIDs, &(pRPDR[lCounter].iArtIDsNum));
            print_string_array(pThisDoc, "QueryKeywords", pRPDR[lCounter].caaSearchKeywords, &(pRPDR[lCounter].iSearchKeywordsNum));
            for (int i = 0; i < pRPDR[lCounter].iArtIDsNum; i++)
            {
                strncpy_s(caUtf8, TEXT_SIZE * 10, pRPDR[lCounter].caaArtIDs[i], _TRUNCATE);
                caUtf8[TEXT_SIZE * 10 - 1] = '\0';
                Utf8ToAnsi(caUtf8, pRPDR[lCounter].caaArtIDs[i], READ_BUF_SIZE);
            }
            for (int i = 0; i < pRPDR[lCounter].iSearchKeywordsNum; i++)
            {
                strncpy_s(caUtf8, TEXT_SIZE * 10, pRPDR[lCounter].caaSearchKeywords[i], _TRUNCATE);
                caUtf8[TEXT_SIZE * 10 - 1] = '\0';
                Utf8ToAnsi(caUtf8, pRPDR[lCounter].caaSearchKeywords[i], READ_BUF_SIZE);
            }
            lCounter++;
        }

        bson_free(pThisStr);
        if(lCounter >= lTotalRecordNum) break;
    }

    mongoc_cursor_destroy(pThisCursor);
    return 0;
}

int UpdateDocuByPromoIDArtIDsQueryKeywordsVisitings(mongoc_collection_t* pmgcolThisCollection, char* caPromoID, \
    char *caInsertedDate, char *caChangerID)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    bson_error_t error;
    char* pThisStr;
    Document ThisDocument;
    char caUtf8[TEXT_SIZE], caThisInsertedDate[READ_BUF_SIZE];


    pThisQuery = bson_new();
    pThisOpts = bson_new();

    // (1) 기본 PromotionID 조건
    std::string utf8_caPromoID = CharArrayToUTF8(caPromoID);
    BSON_APPEND_UTF8(pThisQuery, "PromotionID", utf8_caPromoID.c_str());

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
            AfxMessageBox(_T("FuncLib.cpp-UpdateDocuByPromoIDArtIDsQueryKeywordsVisitings() : JSON이 아니거나 루트가 Object가 아님"));
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return FALSE;
        }

        GetObjectIdFromDocument(pThisDoc, caUtf8, TEXT_SIZE);
        PrintDateFromObjectIdString(caUtf8, caThisInsertedDate, READ_BUF_SIZE);
        strncpy_s(caUtf8, TEXT_SIZE, caThisInsertedDate, _TRUNCATE);
        caUtf8[TEXT_SIZE - 1] = '\0';
        Utf8ToAnsi(caUtf8, caThisInsertedDate, READ_BUF_SIZE);

        if (!strcmp(caThisInsertedDate, caInsertedDate))
        {
            // (1) ObjectId 가져오기
            GetObjectIdFromDocument(pThisDoc, caUtf8, TEXT_SIZE);
            bson_oid_t oid;
            bson_oid_init_from_string(&oid, caUtf8);

            // (2) 업데이트 쿼리 준비
            bson_t* pUpdateFilter = bson_new();
            BSON_APPEND_OID(pUpdateFilter, "_id", &oid);

            bson_t* pUpdateDoc = bson_new();
            bson_t set;
            BSON_APPEND_DOCUMENT_BEGIN(pUpdateDoc, "$set", &set);

            // ① 문자열 필드: ChangerID
            std::string utf8_ChangerID = CharArrayToUTF8(caChangerID);
            BSON_APPEND_UTF8(&set, "ChangerID", utf8_ChangerID.c_str());

            // ② bool 필드: InUse = false
            BSON_APPEND_BOOL(&set, "InUse", false);

            // ③ 날짜 필드: UpdatedDate (현재 시각, ISODate 형식)
            time_t now = time(NULL);
            BSON_APPEND_DATE_TIME(&set, "UpdatedDate", (int64_t)now * 1000);

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
                CString msg;
                msg.Format(_T("문서 업데이트 실패: %S"), error.message);
                AfxMessageBox(msg);
            }
            else
            {
                TRACE("✅ Updated document with _id: %s\n", caUtf8);
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

// 문자열 복사 (strdup 대체용)
char* my_strdup(char* s) {
    char* dup = (char*)malloc(strlen(s) + 1);
    if (dup)
    {
        strncpy_s(dup, strlen(s) + 1, s, _TRUNCATE);
        dup[strlen(s)] = '\0';
    }
    return dup;
}

// 문자열 배열에 추가
void add_part(char*** parts, int* size, int* capacity, char* token) {
    if (*size >= *capacity) {
        *capacity *= 2;
        *parts = (char**)realloc(*parts, sizeof(char*) * (*capacity));
    }
    (*parts)[(*size)++] = my_strdup(token);
}

// URL 분리 함수 (/, ., ?, =, & 모두 구분자로 사용)
char** split_url_all(char* url, int* count) {
    if (!url) return NULL;

    char* url_copy = my_strdup(url);
    if (!url_copy) return NULL;

    int capacity = 20;
    int size = 0;
    char** parts = (char**)malloc(sizeof(char*) * capacity);

    // 1) 프로토콜 추출
    char* protocol_end = strstr(url_copy, "://");
    char* host_start = url_copy;
    if (protocol_end) {
        *protocol_end = '\0';
        add_part(&parts, &size, &capacity, url_copy); // protocol
        host_start = protocol_end + 3;
    }

    // 2) 호스트 전체 추출
    char* path_start = strpbrk(host_start, "/?&="); // host 끝 찾기
    if (path_start) *path_start = '\0';
    add_part(&parts, &size, &capacity, host_start); // full host

    // 3) 호스트 세부 분리 (. 기준)
    char* host_copy = my_strdup(host_start);
    char* next_token = NULL;
    char* token = strtok_s(host_copy, ".", &next_token);
    while (token) {
        add_part(&parts, &size, &capacity, token);
        token = strtok_s(NULL, ".", &next_token);
    }
    free(host_copy);

    // 4) 나머지 부분 분리 (/ ? = & 기준)
    if (path_start) {
        token = strtok_s(path_start + 1, "/?=&", &next_token);
        while (token) {
            add_part(&parts, &size, &capacity, token);
            token = strtok_s(NULL, "/?=&", &next_token);
        }
    }

    *count = size;
    free(url_copy);
    return parts;
}

void keep_digits(char* str) {
    char* src = str;
    char* dst = str;

    while (*src) {
        if (isdigit((unsigned char)*src)) {
            *dst++ = *src;   // 숫자면 복사
        }
        src++;
    }
    *dst = '\0';  // 문자열 끝에 NULL 추가
}

int GetArticleDocuByArticleID(mongoc_collection_t* pmgcolThisCollection, char* caArtID, ArticleDocu* pArticle)
{
    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    bson_error_t error;
    const bson_t* pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    int iCounter = 0;


    std::string utf8_caArtID = CharArrayToUTF8(caArtID);
    pThisQuery = BCON_NEW("ArticleID", BCON_UTF8(utf8_caArtID.c_str()));
    pThisOpts = bson_new();

    // 정렬 및 limit 옵션
    bson_t sort;
    bson_init(&sort);
    BSON_APPEND_INT32(&sort, "_id", -1);
    BSON_APPEND_DOCUMENT(pThisOpts, "sort", &sort);
    BSON_APPEND_INT32(pThisOpts, "limit", 1);

    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);

    if (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pArticle->ClassAxisDim = pArticle->WriterAxisDim = 0;

        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            AfxMessageBox(_T("FuncLib.cpp-GetArticleDocuByArticleID() : JSON이 아니거나 루트가 Object가 아님"));
            bson_free(pThisStr);
            bson_destroy(pThisQuery);
            bson_destroy(pThisOpts);
            bson_destroy(&sort);
            mongoc_cursor_destroy(pThisCursor);
            return FALSE;
        }

        auto ArticleID = ThisDocument.FindMember("ArticleID");
        if ((ArticleID != ThisDocument.MemberEnd()) && (ArticleID->value.IsString()))
        {
            //strncpy(pArticle->ArticleID, ArticleID->value.GetString(), sizeof(pArticle->ArticleID) - strlen(pArticle->ArticleID) - 1);
            strncpy_s(pArticle->ArticleID, READ_BUF_SIZE, ArticleID->value.GetString(), _TRUNCATE);
            pArticle->ArticleID[READ_BUF_SIZE - 1] = '\0';
        }
        auto ClassAxis = ThisDocument.FindMember("ClassAxis");
        if ((ClassAxis != ThisDocument.MemberEnd()) && (ClassAxis->value.IsString()))
        {
            //strncpy(pArticle->ClassAxis, ClassAxis->value.GetString(), sizeof(pArticle->ClassAxis) - strlen(pArticle->ClassAxis) - 1);
            strncpy_s(pArticle->ClassAxis, TEXT_SIZE, ClassAxis->value.GetString(), _TRUNCATE);
            pArticle->ClassAxis[TEXT_SIZE - 1] = '\0';
        }
        auto WriterAxis = ThisDocument.FindMember("WriterAxis");
        if ((WriterAxis != ThisDocument.MemberEnd()) && (WriterAxis->value.IsString()))
        {
            //strncpy(pArticle->WriterAxis, WriterAxis->value.GetString(), sizeof(pArticle->WriterAxis) - strlen(pArticle->WriterAxis) - 1);
            strncpy_s(pArticle->WriterAxis, TEXT_SIZE, WriterAxis->value.GetString(), _TRUNCATE);
            pArticle->WriterAxis[TEXT_SIZE - 1] = '\0';
        }

        bson_iter_t iter;
        if (bson_iter_init_find(&iter, pThisDoc, "RegisteredDate") &&
            BSON_ITER_HOLDS_DATE_TIME(&iter)) {
            int64_t millis = bson_iter_date_time(&iter);
            pArticle->RegisteredDate = millis;   // UTC 밀리초
        }

        int iDummy;
        print_double_array(pThisDoc, "NormalClass", pArticle->NormalClass, &(pArticle->ClassAxisDim));
        print_double_array(pThisDoc, "NormalWriter", pArticle->NormalWriter, &(pArticle->WriterAxisDim));
        print_double_array(pThisDoc, "ArticleAccumEmbedding", pArticle->ArticleAccumEmbedding, &iDummy);
        print_double_array(pThisDoc, "QueryAccumEmbedding", pArticle->QueryAccumEmbedding, &iDummy);

        bson_free(pThisStr);
        iCounter++;
    }
    if (mongoc_cursor_error(pThisCursor, &error)) {
        fprintf(stderr, "Cursor error: %s\n", error.message);
        AfxMessageBox(_T("FuncLib.cpp-GetArticleDocuByArticleID() : Cursor error"));
        bson_destroy(pThisQuery);
        bson_destroy(pThisOpts);
        bson_destroy(&sort);
        mongoc_cursor_destroy(pThisCursor);
        return FALSE;
    }

    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);
    bson_destroy(&sort);
    mongoc_cursor_destroy(pThisCursor);

    if (iCounter > 0)
    {
        return 0;
    }
    else
    {
        return 1001;
    }
}

int GetVisitorDocuNum(mongoc_collection_t* pmgcolThisCollection, char* caCID, char* caName, char* caPhone, \
    char* caMail, char* caID, char* caRecommenders, int* iVisitorDocuNum)
{
    bson_t* query;
    bson_t or_array;
    bson_error_t error;


    char utf8_caCID[NAME_SIZE], utf8_caName[NAME_SIZE], utf8_caPhone[NAME_SIZE], \
        utf8_caMail[NAME_SIZE], utf8_caID[NAME_SIZE], utf8_caRecommenders[NAME_SIZE];
    if (!CharArrayToUTF8(caCID, utf8_caCID, sizeof(utf8_caCID))) 
    {
        AfxMessageBox(_T("caCID의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caName, utf8_caName, sizeof(utf8_caName)))
    {
        AfxMessageBox(_T("caName의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caPhone, utf8_caPhone, sizeof(utf8_caPhone)))
    {
        AfxMessageBox(_T("caPhone의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caMail, utf8_caMail, sizeof(utf8_caMail)))
    {
        AfxMessageBox(_T("caMail의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caID, utf8_caID, sizeof(utf8_caID)))
    {
        AfxMessageBox(_T("caID의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caRecommenders, utf8_caRecommenders, sizeof(utf8_caRecommenders)))
    {
        AfxMessageBox(_T("caRecommenders의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }

    // (3) 쿼리 초기화
    query = bson_new();
    bson_init(&or_array);

    // (4) 입력값이 존재하는 필드만 OR 배열에 추가
    int index = 0;
    char key[16];  // 배열 인덱스 키 (예: "0", "1", "2", ...)

    // ---------------------------------------------------------------------
    // (1) 필드 이름 배열
    const char* fields[7] = {
        "cid",
        "email",
        "name",
        "Phone Number",
        "phone",
        "MemberID",
        "Recommender name"
    };

    // (2) UTF-8 문자열 포인터 배열
    const char* values[7] = {
        utf8_caCID,
        utf8_caMail,
        utf8_caName,
        utf8_caPhone,
        utf8_caPhone,          // Phone과 Phone Number에 동일 적용
        utf8_caID,
        utf8_caRecommenders
    };

    // ---------------------------------------------------------------------
    // (3) 반복문으로 $or 배열 구성
    for (int i = 0; i < 7; i++)
    {
        if (values[i] != NULL && values[i][0] != '\0')
        {
            bson_t child;
            bson_init(&child);

            BSON_APPEND_UTF8(&child, fields[i], values[i]);

            // 🔹 인덱스 문자열 생성 (bson_uint32_to_string 대체)
            _snprintf_s(key, sizeof(key), _TRUNCATE, "%d", index++);

            BSON_APPEND_DOCUMENT(&or_array, key, &child);
            bson_destroy(&child);
        }
    }

    // 조건이 하나도 없으면 0 반환
    if (bson_count_keys(&or_array) == 0)
    {
        *iVisitorDocuNum = 0;
        AfxMessageBox(_T("FuncLib.cpp-GetVisitorDocuNum() : 방문자 검색을 위한 조건이 적어도 하나 이상 있어야만 합니다!"));
        // (7) 메모리 정리
        bson_destroy(query);
        bson_destroy(&or_array);
        return FALSE;
    }

    // (5) $or 조건 추가
    BSON_APPEND_ARRAY(query, "$or", &or_array);
    bson_destroy(&or_array);

    // (6) 문서 개수 카운트
    *iVisitorDocuNum = mongoc_collection_count_documents(
        pmgcolThisCollection,
        query,
        NULL,    // 옵션 없음
        NULL,    // ReadPrefs
        NULL,    // reply 문서 불필요
        &error);

    if (*iVisitorDocuNum < 0)
    {
        fprintf(stderr, "Count failed: %s\n", error.message);
        *iVisitorDocuNum = 0;
        AfxMessageBox(_T("FuncLib.cpp-GetVisitorDocuNum() : Count failed"));
        // (7) 메모리 정리
        bson_destroy(query);
        return FALSE;
    }

    // (7) 메모리 정리
    bson_destroy(query);

    return 0;
}

int GetVisitorDocu(mongoc_collection_t* pmgcolThisCollection, char* caCID, char* caName, char* caPhone, \
    char* caMail, char* caID, char* caRecommenders, VisitorDocu *pVisitors)
{
    bson_t* query;
    bson_t or_array;
    mongoc_cursor_t* cursor;
    const bson_t* doc;
    char* str;
    Document ThisDocument;
    int iCounter = 0;
    char utf8_ca[NAME_SIZE];


    char utf8_caCID[NAME_SIZE], utf8_caName[NAME_SIZE], utf8_caPhone[NAME_SIZE], \
        utf8_caMail[NAME_SIZE], utf8_caID[NAME_SIZE], utf8_caRecommenders[NAME_SIZE];
    if (!CharArrayToUTF8(caCID, utf8_caCID, sizeof(utf8_caCID)))
    {
        AfxMessageBox(_T("caCID의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caName, utf8_caName, sizeof(utf8_caName)))
    {
        AfxMessageBox(_T("caName의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caPhone, utf8_caPhone, sizeof(utf8_caPhone)))
    {
        AfxMessageBox(_T("caPhone의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caMail, utf8_caMail, sizeof(utf8_caMail)))
    {
        AfxMessageBox(_T("caMail의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caID, utf8_caID, sizeof(utf8_caID)))
    {
        AfxMessageBox(_T("caID의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }
    if (!CharArrayToUTF8(caRecommenders, utf8_caRecommenders, sizeof(utf8_caRecommenders)))
    {
        AfxMessageBox(_T("caRecommenders의 UTF8 변환에 실패했습니다!"));
        return FALSE;
    }

    // (3) 쿼리 초기화
    query = bson_new();
    bson_init(&or_array);

    // (4) 입력값이 존재하는 필드만 OR 배열에 추가
    int index = 0;
    char key[16];  // 배열 인덱스 키 (예: "0", "1", "2", ...)

    // ---------------------------------------------------------------------
    // (1) 필드 이름 배열
    const char* fields[7] = {
        "cid",
        "email",
        "name",
        "Phone Number",
        "phone",
        "MemberID",
        "Recommender name"
    };

    // (2) UTF-8 문자열 포인터 배열
    const char* values[7] = {
        utf8_caCID,
        utf8_caMail,
        utf8_caName,
        utf8_caPhone,
        utf8_caPhone,          // Phone과 Phone Number에 동일 적용
        utf8_caID,
        utf8_caRecommenders
    };

    // ---------------------------------------------------------------------
    // (3) 반복문으로 $or 배열 구성
    for (int i = 0; i < 7; i++)
    {
        if (values[i] != NULL && values[i][0] != '\0')
        {
            bson_t child;
            bson_init(&child);

            BSON_APPEND_UTF8(&child, fields[i], values[i]);

            // 🔹 인덱스 문자열 생성 (bson_uint32_to_string 대체)
            _snprintf_s(key, sizeof(key), _TRUNCATE, "%d", index++);

            BSON_APPEND_DOCUMENT(&or_array, key, &child);
            bson_destroy(&child);
        }
    }

    // OR 배열에 조건이 하나도 없으면 리턴
    if (bson_count_keys(&or_array) == 0)
    {
        printf("No valid search parameters provided.\n");
        bson_destroy(query);
        bson_destroy(&or_array);
        AfxMessageBox(_T("FuncLib.cpp-GetVisitorDocu() : No valid search parameters provided."));
        return FALSE;
    }

    // (5) $or 조건 추가
    BSON_APPEND_ARRAY(query, "$or", &or_array);
    bson_destroy(&or_array);

    // (6) 쿼리 실행
    cursor = mongoc_collection_find_with_opts(pmgcolThisCollection, query, NULL, NULL);

    // (7) 결과 출력
    while (mongoc_cursor_next(cursor, &doc))
    {
        str = bson_as_json(doc, NULL);
        ThisDocument.Parse(str);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            AfxMessageBox(_T("FuncLib.cpp-GetVisitorDocu() : JSON이 아니거나 루트가 Object가 아님"));
            bson_free(str);
            bson_destroy(query);
            mongoc_cursor_destroy(cursor);
            return FALSE;
        }

        auto cid = ThisDocument.FindMember("cid");
        if ((cid != ThisDocument.MemberEnd()) && (cid->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, cid->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caCID, NAME_SIZE);
        }
        auto email = ThisDocument.FindMember("email");
        if ((email != ThisDocument.MemberEnd()) && (email->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, email->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caMail, NAME_SIZE);
        }
        auto name = ThisDocument.FindMember("name");
        if ((name != ThisDocument.MemberEnd()) && (name->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, name->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caName, NAME_SIZE);
        }
        auto PhoneNumber = ThisDocument.FindMember("Phone Number");
        if ((PhoneNumber != ThisDocument.MemberEnd()) && (PhoneNumber->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, PhoneNumber->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caPhone, NAME_SIZE);
        }
        auto phone = ThisDocument.FindMember("phone");
        if ((phone != ThisDocument.MemberEnd()) && (phone->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, phone->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            char ansi_ca[NAME_SIZE];
            Utf8ToAnsi(utf8_ca, ansi_ca, NAME_SIZE);
            strncat_s(pVisitors[iCounter].caPhone, NAME_SIZE, ", ", _TRUNCATE);
            strncat_s(pVisitors[iCounter].caPhone, NAME_SIZE, ansi_ca, _TRUNCATE);
        }
        auto MemberID = ThisDocument.FindMember("MemberID");
        if ((MemberID != ThisDocument.MemberEnd()) && (MemberID->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, MemberID->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caID, NAME_SIZE);
        }
        auto Recommender = ThisDocument.FindMember("Recommender name");
        if ((Recommender != ThisDocument.MemberEnd()) && (Recommender->value.IsString()))
        {
            strncpy_s(utf8_ca, NAME_SIZE, Recommender->value.GetString(), _TRUNCATE);
            utf8_ca[NAME_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pVisitors[iCounter].caRecommenders, NAME_SIZE);
        }

        iCounter++;
        bson_free(str);
    }

    if (mongoc_cursor_error(cursor, NULL))
    {
        fprintf(stderr, "Cursor error occurred.\n");
        AfxMessageBox(_T("FuncLib.cpp-GetVisitorDocu() : Cursor error occurred."));
        // (8) 메모리 정리
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        return FALSE;
    }

    // (8) 메모리 정리
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return 0;
}

int GetAnalDocuByCID(mongoc_collection_t* pmgcolThisCollection, char* caCID, AnalDocu* pAnal)
{
    pAnal->ClassAxisDim = pAnal->WriterAxisDim = 0;

    mongoc_cursor_t* pThisCursor;
    bson_t* pThisQuery, * pThisOpts;
    const bson_t* pThisDoc;
    char* pThisStr;
    Document ThisDocument;
    char utf8_ca[TEXT_SIZE], caThisInsertedDate[READ_BUF_SIZE];
    int iCounter = 0;


    pThisQuery = BCON_NEW("cid", BCON_UTF8(caCID));
    pThisOpts = BCON_NEW("sort", "{", "_id", BCON_INT32(-1), "}"); // 역순
    //    pThisOpts = bson_new();
        //pThisQuery = bson_new();
        //BSON_APPEND_UTF8 (pThisQuery, "hello", "world");
        //pThisOpts = BCON_NEW ("cid", BCON_INT32(1), "email", BCON_INT32(1));
        //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, pThisOpts, NULL);
        //pThisCursor = mongoc_collection_find_with_opts (pmgcolThisCollection, NULL, NULL, NULL);
    pThisCursor = mongoc_collection_find_with_opts(pmgcolThisCollection, pThisQuery, pThisOpts, NULL);
    bson_destroy(pThisQuery);
    bson_destroy(pThisOpts);

    while (mongoc_cursor_next(pThisCursor, &pThisDoc))
    {
        pThisStr = bson_as_json(pThisDoc, NULL);
        ThisDocument.Parse(pThisStr);
        if (ThisDocument.HasParseError() || !ThisDocument.IsObject()) {
            // JSON이 아니거나 루트가 Object가 아님 → 여기서 리턴/기본값 세팅
            bson_free(pThisStr);
            return 400;
        }

        auto cid = ThisDocument.FindMember("cid");
        if ((cid != ThisDocument.MemberEnd()) && (cid->value.IsString()))
        {
            strncpy_s(utf8_ca, TEXT_SIZE, cid->value.GetString(), _TRUNCATE);
            utf8_ca[TEXT_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pAnal->cid, READ_BUF_SIZE);
        }
        auto ClassAxis = ThisDocument.FindMember("ClassAxis");
        if ((ClassAxis != ThisDocument.MemberEnd()) && (ClassAxis->value.IsString()))
        {
            strncpy_s(utf8_ca, TEXT_SIZE, ClassAxis->value.GetString(), _TRUNCATE);
            utf8_ca[TEXT_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pAnal->ClassAxis, TEXT_SIZE);
        }
        auto WriterAxis = ThisDocument.FindMember("WriterAxis");
        if ((WriterAxis != ThisDocument.MemberEnd()) && (WriterAxis->value.IsString()))
        {
            strncpy_s(utf8_ca, TEXT_SIZE, WriterAxis->value.GetString(), _TRUNCATE);
            utf8_ca[TEXT_SIZE - 1] = '\0';
            Utf8ToAnsi(utf8_ca, pAnal->WriterAxis, TEXT_SIZE);
        }
        auto TotalEventSize = ThisDocument.FindMember("TotalEventSize");
        if ((TotalEventSize != ThisDocument.MemberEnd()) && (TotalEventSize->value.IsInt64()))
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

        GetObjectIdFromDocument(pThisDoc, utf8_ca, TEXT_SIZE);
        PrintDateFromObjectIdString(utf8_ca, caThisInsertedDate, READ_BUF_SIZE);
        strncpy_s(utf8_ca, TEXT_SIZE, caThisInsertedDate, _TRUNCATE);
        utf8_ca[TEXT_SIZE - 1] = '\0';
        Utf8ToAnsi(utf8_ca, pAnal->RegistedDate, READ_BUF_SIZE);

        iCounter++;

        bson_free(pThisStr);
        break;
    }

    mongoc_cursor_destroy(pThisCursor);
    if (iCounter > 0)
    {
        return 0;
    }
    else
    {
        return 1001;
    }
}

