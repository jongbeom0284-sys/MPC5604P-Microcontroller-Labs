# 작업 출처와 공개 범위

국민대학교 3학년 2학기 마이크로프로세서응용의 수업 작업본을 정리했습니다.

| 공개 파일 | 원본과의 관계 |
|---|---|
| `src/main.c` | 로컬 `Example_Base_5604/src/main.c`. 보관된 기본 예제 압축본의 짧은 초기 main과 달리 ADC·PIT·PWM·DSPI·eTimer·전류/속도 제어 실습이 추가된 작업본입니다. |
| `labs/adc-gpio/main.c` | 같은 폴더의 `src/test of main.c`를 실습 단계가 드러나는 이름으로 옮긴 사본입니다. |
| `patches/uart-19200.patch` | 기본 예제 대비 `src/init_base.c`의 baud rate 선택 변경분입니다. 기반 초기화 파일 전체는 복사하지 않았습니다. |
| `freemaster/Example.pmp` | 기본 예제와 다른 로컬 FreeMASTER 설정 파일을 바이너리 그대로 보존했습니다. 코드·빌드 결과를 대신하는 파일은 아닙니다. |
| `assets/motor-current-response.png` | 본인 명의로 제출한 DC 모터 과제 보고서의 전류 측정 결과 화면입니다. |
| `assets/speed-control-report.png`, `assets/current-control-report.png` | 같은 보고서의 속도·전류 제어 코드 설명입니다. |
| `assets/spi-master-report.png` | 2인 SPI 통신 과제 보고서의 Master 노드 설명입니다. 개인별 구현 분담은 자료만으로 확정하지 않았습니다. |

기본 프로젝트의 Freescale 장치 헤더·startup·FreeMASTER 통신 라이브러리·런타임과 수업 CAN/SPI 배포 예제는 본인 단독 작성 코드로 공개하지 않았습니다. 설치 프로그램, 드라이버, 강의자료, 시험자료, 학생 번호가 있는 원본 보고서도 포함하지 않았습니다.

`src/main.c`의 CP949 한글 주석은 UTF-8로 변환했고, 소스의 줄바꿈·행 끝 공백만 정리했습니다. 제어 로직과 실습 당시 남긴 주석은 보존했습니다. 원본 폴더의 파일은 수정하지 않았습니다.

이 저장소는 수업을 기반으로 한 학습 기록입니다. 사용한 예제·SDK·장치 정의의 권리와 사용 조건은 각각의 원본에 따르며, 전체 구성에 새로운 단일 라이선스를 부여하지 않습니다.
