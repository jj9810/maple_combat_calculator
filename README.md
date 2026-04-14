# Maple Combat Calculator (MCC)

메이플스토리의 전투력을 정교하게 계산하고, 스킬 타임라인 기반의 시뮬레이션을 수행하는 C++ 엔진입니다. 넥슨 오픈 API 데이터와 로컬 사용자 정의 룰을 모두 지원합니다.

## 개요
이 프로젝트는 복잡한 데미지 공식과 대량의 전투 로그를 빠르고 정확하게 처리하기 위해 설계되었습니다. 모든 핵심 연산 로직은 C++로 작성되었으며, 향후 WebAssembly(Wasm)로 컴파일되어 웹 브라우저에서도 실행될 수 있도록 확장성을 고려하여 설계되었습니다.

## 핵심 엔진 컴포넌트

엔진은 책임 범위에 따라 네 가지 주요 패키지로 나뉩니다.

### MDC (Maple Data Collector)
- **역할**: 사용자 정의 룰(Policy) 및 컴포넌트 데이터를 기반으로 가상 스킬 타임라인을 생성하는 패키지입니다.
- **주요 기능**:
  - YAML 기반의 보스/몬스터 데이터 및 스킬 프리셋 관리.
  - 룰 기반 행동 결정 시뮬레이션 및 타임라인 생성 엔진(`mdc::Engine`).
- **진행 상황**: 
  - YAML 기반 캐릭터/몬스터/스킬/정책 로더 구현 완료.
  - 시뮬레이션 엔진 및 엔티티 시스템 구축 완료.
- **TODO**:
  - 더 복잡한 조건문(Conditional Actions) 지원을 위한 Policy 엔진 고도화.
  - 공식 API의 상세 장비/스탯 데이터를 기반으로 한 정밀 입력 모듈 개발.

### MCM (Maple Combat Manager)
- **역할**: 프로젝트의 메인 실행 포인트입니다. 외부(nexon_api) 또는 로컬(MDC)에서 전달받은 타임라인을 실행하고 최종 전투 결과(Combat Power, Damage Report)를 산출합니다.
- **주요 기능**:
  - 캐릭터 시뮬레이션을 수행하고 전투 컨텍스트를 제어합니다.
  - `Aggregator`를 통한 캐릭터/장비/스킬 스탯의 단계별 집계 및 결과 리포트 출력.
  - MCC를 호출하여 최종 계산을 수행하는 오케스트레이터 역할.
- **진행 상황**:
  - 타임라인 주입 방식의 `calculate` 인터페이스 구축 및 데미지 집계 로직 구현 완료.
  - 보스 데이터(`MonsterInfo`) 독립 로드 로직 구현 완료.
- **TODO**:
  - 타임라인 기반의 버프 가동률 및 딜 로스(Loss) 분석 기능 추가.
  - 멀티 스레딩 기반의 대량 시뮬레이션 지원.

### MCC (Maple Combat Calculator)
- **역할**: 주어진 정보를 바탕으로 실제 전투력(Combat Power) 및 스킬 기대 데미지를 산출하는 순수 연산 라이브러리입니다.
- **주요 기능**:
  - 캐릭터의 종합 전투력 계산.
  - 특정 조건(방어율 무시, 속성 반감, 포스 보정 등)에 따른 스킬 기대 데미지 도출.
- **진행 상황**:
  - 전투력(`calculateCombatPower`) 및 숙련도/맥뎀 보정이 반영된 스킬 데미지 계산 로직 구현 완료.
- **제약 사항 (현재 미지원)**:
  - 데몬어벤져 (HP 기반 주스탯)
  - 제논 (다중 주스탯)
- **TODO**:
  - `BattlePracticeResult`를 정답셋(Ground Truth)으로 활용하여 계산 공식의 오차 자동 검증 및 보정(Calibration).
  - 공식 API의 실제 누적 데미지와 엔진 계산값 간의 비교 분석 모듈 구현.

### nexon_api (Maple Nexon API)
- **역할**: 넥슨 오픈 API와 직접 통신하여 공식 캐릭터 정보 및 연무장 타임라인 데이터를 수집하고 Protobuf 객체로 정형화하는 패키지입니다.
- **주요 기능**:
  - `libcurl` 기반의 HTTP 통신 및 API 키 인증 관리.
  - JSON 응답을 `shared/proto` 규격의 Protobuf 메시지로 자동 변환.
- **진행 상황**:
  - 기본 통신 골격 및 `NexonApiLoader` 구현 완료.
  - `SkillTimeline`, `BattlePracticeCharacterInfo` 등 주요 데이터 로더 구축 완료.
- **TODO**:
  - API 호출 결과 캐싱 로직 추가.
  - 다양한 API 엔드포인트(장비 정보 등) 확장 지원.

## 시나리오
Maple Combat Calculator는 Nexon API가 제공하는 공식 연무장 타임라인 기반 시뮬레이션과 로컬에서 정의된 룰 기반 시뮬레이션을 모두 지원합니다.

### 넥슨 공식 API 타임라인 플로우
넥슨 API에서 제공하는 전투 기록을 바탕으로 데미지를 산출합니다.

- 입력: 넥슨 Open API에서 제공하는 SkillTimeline (전투 로그) + BattlePracticeCharacterInfo (캐릭터 상세 정보), 로컬 설정 파일(data/)에 정의된 보스 프리셋.
- 플로우:
  - 사용자가 외부에서 획득한 두 프로토콜 버퍼 메시지를 **MCM::calculate()** 함수에 주입합니다.
  - MCM은 제공된 타임라인을 순회하며 각 시점의 스탯을 계산하고 MCC 수식을 호출하여 데미지를 산출합니다.
- 최종 결과: 집계된 데미지 정보가 담긴 BattlePracticeResult를 반환합니다.

### 로컬 룰 기반 시뮬레이션 플로우
사용자가 정의한 룰에 따라 가상의 전투를 수행하여 타임라인을 생성한 후, MCM을 통해 데미지를 산출합니다.

- 입력: 로컬 설정 파일(data/)에 정의된 직업/보스/로테이션 정책(Policy) 및 캐릭터 프리셋.
- 플로우:
  - MDC::Loader가 로컬 설정 파일들을 읽어 캐릭터 정보와 내부 룰 객체들을 준비합니다.
  - **MDC::Engine::run()**이 실행되어 설정된 Policy(룰)에 따라 매 프레임마다 행동을 결정하고, **가상의 SkillTimeline**을 생성하여 반환합니다.
  - 생성된 타임라인과 캐릭터 정보를 **MCM::calculate()**에 주입하여 실제 전투 결과를 연산합니다.
- 최종 결과: MDC가 생성한 SkillTimeline과 이를 바탕으로 MCM이 산출한 BattlePracticeResult를 얻습니다.

## 데이터 규격 (Protobuf)

시스템 간의 데이터 교환 및 엔진 내부 데이터 구조의 일관성을 위해 **Google Protocol Buffers**를 사용합니다.

- 정의 위치: `shared/proto/`
- 주요 메시지:
  - `battle_practice_character_info.proto`: 넥슨 Open API 캐릭터 사양 규격
  - `battle_practice_result.proto`: 연무장 전투 결과 데이터
  - `skill_timeline.proto`: 스킬 사용 기록 타임라인 규격

## 프로젝트 구조

```text
/
├── mcc/               # Core Calculator (전투력/스킬 데미지 엔진)
├── mcm/               # Combat Manager (데이터 집계 및 컨텍스트 관리)
├── mdc/               # Data Generator (가상 타임라인 생성 엔진)
├── nexon_api/         # Nexon API Loader (외부 데이터 수집기)
├── shared/            # 공통 코드 (Proto 등)
└── data/              # YAML 프리셋 (Jobs, Mobs, Policies)
```

## 참고 및 인용 자료 (References)

MCC 연산 엔진의 수식 및 로직은 다음 자료를 기반으로 구현되었습니다.
- [메이플스토리 인벤: 전투력 공식 분석](https://www.inven.co.kr/board/maple/2299/5679951)
- [oleneyl/maplestory_dpm_calc (GitHub)](https://github.com/oleneyl/maplestory_dpm_calc)

## 개발 및 빌드

### 요구 사항
- C++ 20 이상 호환 컴파일러
- [CMake](https://cmake.org/) (3.10 이상)
- [Emscripten SDK](https://emscripten.org/) (Wasm 빌드 시 필요)
- [Protobuf Compiler (protoc)](https://protobuf.dev/)

### 로컬 빌드 (C++ 테스트용)
```bash
mkdir build && cd build
cmake ..
make
# MCM 실행 (메인 포인트)
./mcm/mcm_runner
# Nexon API 테스트
./nexon_api/nexon_api_test
```

## 라이선스 (License)

이 프로젝트는 **GNU Affero General Public License v3.0 (AGPL-3.0)** 을 따릅니다.

## AI 어시스턴트

이 프로젝트는 **Gemini CLI**를 사용하여 개발하고 있습니다.
