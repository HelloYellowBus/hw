# README

 이 문서는 LPWA 기술을 활용한 하드웨어 및 서비스 개발을 주제로 한 [서울 하드웨어 해커톤](https://www.seoulhackathon.org/538) 에 참여한 HelloYellowBus팀의 H/W 및 S/W 코드를 정리한 내용



[TOC]

## HelloYellowBus 

### 서비스 소개

Arm mbed에 내장된 GPS, 온도 센서, 승/하차 버튼을 활용해 학생들이 통학버스 승하차시 터치하는 방식의 하드웨어와 전송된 정보를 실시간으로 모니터링 할 수 있는 웹 애플리케이션 서비스

- 통학버스 안전사고 문제에 있어, 저비용으로 효과적인 예방책 및 해결책 제시
- 기존 차량의 구조적인 변화 없이 기존 교육부이 도입한 시스템에 비해 실효성 측면에서 경쟁력 확보



### 구조도 

![gozodo]()

## 개발일정

- 기술 워크숍 (19.12.14 ~ 12.29)

  - 1주차

    - arm Mbed의 기본 센서 활용  
    - wifi 망를 이용한 Pelion(Cloud system)과의 연동

  - 2주차

    - LPWA 기술 활용을 위한 통신 쉴드(NB-IoT, Cat.M1) 사용법
    - cellular 망을 이용한 Pelion과의 연동 

  - 3주차 : 클라우드 (The-K system) 

    - http을 이용한 The-k system과의 연동

      

- 하드웨어 해커톤 (20.1.4 ~ 20.1.5)

  - H/W

    - 승/하차를 위한 Button 연결
    - GPS 연결 및 위치 정보 수집
    - Cat.M1 모듈을 활용해, mbed 보드와 Cloud(Things park, Pelion) Cellular로 연결   

  - S/W

    - HelloYellowBus 웹 애플리케이션 기본 기능 구현
      - 사용자 인증(firebase authentication)
      - 구글맵 api를 이용해 하드웨어에서 수집한 위치 및 센서 값 시각화
      - 통학버스차량 근접시 진동알림

    - REST API를 통한 Cloud(Things park, Pelion)와 앱과 연동 시도

      - Pelion과 연동시 겪은 문제점

        Pelion의 실시간 response를 받는 전용 서버 구축이 필요.

        ( Django [`StreamingHttpResponse`](https://docs.djangoproject.com/en/3.0/ref/request-response/#django.http.StreamingHttpResponse) 객체로 구현 가능)

      - Things Park

        Things Park에 response를 받는 전용 서버 구축이 필요(CORS Error)



## H/W와 S/W repository의 주요 코드

### H/W repo



이 코드는 `Pelion Device Ready example` 를 기반으로 작성했습니다. 

[띵스파크 http 연동 참고 코드](https://os.mbed.com/users/master_k1/code/thingspark-example/)





### S/W repo





- 

