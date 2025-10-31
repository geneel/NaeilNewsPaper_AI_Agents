본 프로젝트의 대부분 소스코드는 MongoDB와 MySQL 접속이 필요하다.
따라서 이 소스코드들을 컴파일하기 위해서는 먼저 MongoDB와 MySQL용 헤더파일들과 라이브러리를 컴파일러에서 설정해야만 한다.



[VisitorAnalyzer]
MongoDB의 Visitors 컬렉션과 Events 컬렉션에 저당된 도큐먼트들을 읽어와서 분석한 다음 그 결과를 Anals 컬렉션에 저장한다.

[ArticleAnalyzer]
MySQL에서 기사 정보를 읽어와서 분석한 다음 그 결과를  MongoDB의 Articles 컬렉션에 저장한다.

[PromotionAnalyzer]
PromotionRegister를 통하여 MongoDB의 Promotions 컬렉션에 저장된 정보와 MySQL에서 읽어 온 기사 정보들과 프로모션 정보들과 비교하여 필요하다면 이를 갱신한다.

[PromotionRegister]
내일신문의 방문자들을 구독자로 이끌기 위한 프로모션을 등록할 수 있는 프로그램이다.
해당 프로모션이 효과가 있을 것으로 기대되는 방문자의 방문패턴, 방문자들이 선호할 만한 기사들, 방문자들이 선호할 만한 검색어 등을 입력한다.
그 외에도 각 방문자들에 대해서 AI가 분석한 결과값, 기사들에 대해서 AI가 분석한 결과값, 프로모션들에 대해서 AI가 분석한 결과값을 조회할 수 있다.

[ArticleRecommender]
입력받은 방문자의 고유 아이디(cid) 또는 내일신문 홈페이지 로그인 아이디(MEM_ID)가 가장 선호할 만한 기사들의 고유키(ART_ID)를 순서대로 리턴한다.
이 데몬(서비스, 컴포넌트)을 이용하여 내일신문 홈페이지(PC, Mobile)와 앱(iOS, Android)의 "AI 추천 기사 서비스"와 "개인화된 카카오 알림톡"을 구현할 수 있다.

[PromotionRecommender]
입력받은 방문자의 고유 아이디(cid) 또는 내일신문 홈페이지 로그인 아이디(MEM_ID)를 구독자로 이끌 확률이 가장 높은 프로모션들의 고유키(BNNR_SEQ)를 순서대로 리턴한다.
이 데몬(서비스, 컴포넌트)을 이용하여 내일신문 홈페이지(PC, Mobile)와 앱(iOS, Android)의 "가장 성공확률이 높은 구독 유인 프로모션"를 구현할 수 있다.

