
// PromotionRegister.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif


#include "resource.h"		// 주 기호입니다.
#include "FuncLib.h"
#include "CLoginDlg.h"


// CPromotionRegisterApp:
// 이 클래스의 구현에 대해서는 PromotionRegister.cpp을(를) 참조하세요.
//

class CPromotionRegisterApp : public CWinApp
{
public:
	CPromotionRegisterApp();

// 재정의입니다.
    char caBigDataServerIP[READ_BUF_SIZE];
    char caDBServerIP[READ_BUF_SIZE];

    char BigData_uri_string[STR_SIZE];  // "mongodb://localhost:37017"
    mongoc_uri_t* BigData_uri;
    mongoc_client_t* BigData_client;
    mongoc_database_t* BigData_database;
    mongoc_collection_t * BigData_Visitors_collection, * BigData_Anals_collection, * BigData_Articles_collection, \
        * BigData_Promotions_collection, * BigData_Users_collection;
    bson_error_t BigData_error;

    MYSQL* DB_conn, DB_connection;
    MYSQL_RES* DB_result;
    MYSQL_ROW DB_row;
    char DB_HOST[NAME_SIZE];    //호스트명
    char DB_USER[NAME_SIZE];    //사용자명
    char DB_PASS[NAME_SIZE];    //비밀번호
    char DB_NAME[NAME_SIZE];    //DB명

    bool m_Login;
    char m_User[READ_BUF_SIZE];


public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();
};

extern CPromotionRegisterApp theApp;
