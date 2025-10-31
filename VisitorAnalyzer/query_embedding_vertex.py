#/*
# * To change this license header, choose License Headers in Project Properties.
# * To change this template file, choose Tools | Templates
# * and open the template in the editor.
# */
#
#/* 
# * Copyright
# * File:   query_embedding_vertex.py
# * Author: 이해성, geneel@me.com; (주)내일이비즈; (주)내일신문
# *
# * Created on 2025년 7월 1일 (화), 오전 12:35
#
#Licensed under the Apache License, Version 2.0 (the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.
# */

import sys
import json
import re
# from vertexai.language_models import TextEmbeddingModel
# import vertexai
from google.oauth2 import service_account
from google.auth.transport.requests import Request
from google import generativeai as genai
# from google.generativeai import GenerativeModel
from google.generativeai import types

MAX_TEXT_BYTES = 30000  # 30KB

def truncate_utf8(text, max_bytes):
    encoded = text.encode("utf-8")
    if len(encoded) <= max_bytes:
        return text
    truncated = encoded[:max_bytes]
    return truncated.decode("utf-8", errors="ignore")  # 유효하지 않은 UTF-8 문자는 제거됨

def sanitize_input(text):
    # surrogate 제거 (U+D800–U+DFFF)
    text = re.sub(r'[\uD800-\uDFFF]', '', text)
    # 제어문자 제거
    text = ''.join(ch for ch in text if ch.isprintable())
    return text


genai.configure(api_key="")


def main():
    # project = "naeilcrm"
    # location = "asia-northeast3"
    # vertexai.init(project=project, location=location)

    # C 프로그램에서 받은 텍스트 (표준입력 또는 인자)
    if len(sys.argv) < 2:
        print("No input text.")
        return
    text = sys.argv[1]
    text = truncate_utf8(text, MAX_TEXT_BYTES)
    text = sanitize_input(text)

    # 임베딩 요청 (전역 함수 사용)
    result = genai.embed_content(
        model="models/embedding-001",
        content=text,
        task_type="RETRIEVAL_QUERY",  # 또는 "RETRIEVAL_DOCUMENT"
#        task_type="RETRIEVAL_DOCUMENT",  # 또는 "RETRIEVAL_QUERY"
    )

    # 벡터 추출 및 출력
    vector = result["embedding"]
    # print(json.dumps(vector))
    # print(len(result["embedding"]))  # → 768
    print(result)

if __name__ == "__main__":
    main()

