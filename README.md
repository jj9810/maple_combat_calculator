# Maple Combat Calculator (MCC)

메이플스토리의 전투력을 정교하게 계산하고, 스킬 타임라인 기반의 시뮬레이션을 수행하는 C++ 엔진입니다. 넥슨 오픈 API 데이터와 로컬 사용자 정의 룰을 모두 지원하며, **WebAssembly(Wasm)를 통해 웹 브라우저에서도 고성능 시뮬레이션이 가능합니다.**

## 개요
이 프로젝트는 복잡한 데미지 공식과 대량의 전투 로그를 빠르고 정확하게 처리하기 위해 설계되었습니다. 모든 핵심 연산 로직은 C++로 작성되었으며, Emscripten을 통해 WebAssembly로 빌드되어 웹 브라우저에서도 실행될 수 있도록 설계되었습니다.

## 핵심 엔진 컴포넌트

엔진은 책임 범위에 따라 네 가지 주요 패키지로 나뉩니다.

### MCM (Maple Combat Manager)
- **역할**: 프로젝트의 메인 실행 포인트입니다. 외부 데이터 또는 로컬 프리셋을 기반으로 시뮬레이션을 수행하고 최종 전투 결과(Damage Report)를 산출합니다.
- **주요 기능**:
  - 캐릭터 시뮬레이션을 수행하고 전투 컨텍스트를 제어합니다.
  - `LogLoader`를 통해 JSON 문자열 또는 바이너리 로그를 Protobuf 객체로 변환.
  - **WebAssembly 지원**: Embind를 통해 자바스크립트에서 직접 시뮬레이션 함수(`calculateBattleReport`) 호출 가능.
- **진행 상황**:
  - **[완료]** 타임라인 주입 방식의 `calculate` 인터페이스 구축 및 데미지 집계 로직 구현.
  - **[완료]** WebAssembly 가상 파일 시스템 적용 (브라우저 내 YAML 데이터 로드 지원).
- **TODO**:
  - 타임라인 기반의 버프 가동률 및 딜 로스(Loss) 분석 기능 추가.

### MCC (Maple Combat Calculator)
- **역할**: 실제 전투력(Combat Power) 및 스킬 기대 데미지를 산출하는 순수 연산 라이브러리입니다.
- **주요 기능**:
  - 캐릭터의 종합 전투력 계산.
  - `boost::multiprecision` 기반의 고정밀 데미지 연산 및 맥뎀 보정(Max Damage Correction) 지원.
- **진행 상황**:
  - 전투력(`calculateCombatPower`) 산출 및 `boost::multiprecision` 기반의 고정밀 데미지 연산 로직 구현 완료.
  - 숙련도와 맥뎀 보정(Max Damage Correction)이 반영된 정교한 기대값 산출 지원.
- **제약 사항 (현재 미지원)**:
  - 데몬어벤져 (HP 기반 주스탯), 제논 (다중 주스탯)

### nexon_api (Maple Nexon API)
- **역할**: 넥슨 오픈 API와 통신하여 실시간 데이터를 수집하는 패키지입니다.
- **주요 기능**:
  - `libcurl` 기반의 HTTP 통신 및 API 키 인증 관리.
- **진행 상황**:
  - **[완료]** 기본 통신 및 데이터 수집 모듈 구축.
  - **참고**: WebAssembly 빌드 시에는 네트워크 보안 정책 및 경량화를 위해 제외됩니다.
- **TODO**:
  - API 호출 결과 캐싱 로직 추가.
  - 다양한 API 엔드포인트(장비 정보 등) 확장 지원.

### MDC (Maple Data Collector)
- **역할**: 사용자 정의 룰(Policy) 및 컴포넌트 데이터를 기반으로 가상 스킬 타임라인을 생성하는 패키지입니다.
- **주요 기능**:
  - YAML 기반의 보스/몬스터 데이터 및 스킬 프리셋 관리.
  - 룰 기반 행동 결정 시뮬레이션 및 타임라인 생성 엔진(`mdc::Engine`).
- **진행 상황**: 
  - **[보류]** 작업량 과다로 인해 개발 우선순위 조정 (MCM/MCC 우선 집중).
  - (기존 구현분) YAML 기반 캐릭터/몬스터/스킬/정책 로더 및 시뮬레이션 엔진 골격.
- **TODO**:
  - 더 복잡한 조건문(Conditional Actions) 지원을 위한 Policy 엔진 고도화.
  - 공식 API의 상세 장비/스탯 데이터를 기반으로 한 정밀 입력 모듈 개발.


## 시나리오
Maple Combat Calculator는 Nexon API가 제공하는 공식 연무장 타임라인 기반 시뮬레이션과 로컬에서 정의된 룰 기반 시뮬레이션을 모두 지원합니다.

### 넥슨 공식 API 타임라인 플로우
넥슨 API에서 제공하는 전투 기록을 바탕으로 데미지를 산출합니다.

- 입력: 넥슨 Open API에서 제공하는 SkillTimeline (전투 로그) + BattlePracticeCharacterInfo (캐릭터 상세 정보), 로컬 설정 파일(shared/data/)에 정의된 보스 프리셋.
- 플로우:
  - 사용자가 획득한 데이터를 **MCM::calculateBattleReport()** (Wasm) 또는 **MCM::calculate()** (CLI) 함수에 주입합니다.
  - MCM은 제공된 타임라인을 순회하며 각 시점의 스탯을 계산하고 MCC 수식을 호출하여 데미지를 산출합니다.
- 최종 결과: 집계된 데미지 정보가 담긴 JSON 리포트 또는 Protobuf 메시지를 반환합니다.

### 로컬 룰 기반 시뮬레이션 플로우
사용자가 정의한 룰에 따라 가상의 전투를 수행하여 타임라인을 생성한 후, MCM을 통해 데미지를 산출합니다.

- 입력: 로컬 설정 파일(shared/data/)에 정의된 직업/보스/로테이션 정책(Policy) 및 캐릭터 프리셋.
- 플로우:
  - MDC::Loader가 로컬 설정 파일들을 읽어 캐릭터 정보와 내부 룰 객체들을 준비합니다.
  - **MDC::Engine::run()**이 실행되어 설정된 Policy(룰)에 따라 매 프레임마다 행동을 결정하고, **가상의 SkillTimeline**을 생성하여 반환합니다.
  - 생성된 타임라인과 캐릭터 정보를 **MCM::calculate()**에 주입하여 실제 전투 결과를 연산합니다.
- 최종 결과: MDC가 생성한 SkillTimeline과 이를 바탕으로 MCM이 산출한 BattlePracticeResult를 얻습니다.

## 데이터 규격 (Protobuf)

시스템 간의 데이터 교환 및 엔진 내부 데이터 구조의 일관성을 위해 **Google Protocol Buffers**를 사용합니다.

- 정의 위치: `shared/proto/`
  - `nexon/`: 넥슨 Open API 공식 규격 (CharacterInfo, Result, Timeline 등)
  - `internal/`: 엔진 내부 시뮬레이션 및 연산용 규격 (CombatLog, MCCStat 등)
- 주요 메시지:
  - `nexon/battle_practice_character_info.proto`: 넥슨 Open API 캐릭터 사양 규격
  - `nexon/battle_practice_result.proto`: 연무장 전투 결과 데이터
  - `nexon/skill_timeline.proto`: 스킬 사용 기록 타임라인 규격
  - `internal/combat_log.proto`: 엔진 내부 통합 로그 규격

## 프로젝트 구조

```text
/
├── mcc/               # Core Calculator (전투력/스킬 데미지 엔진)
├── mcm/               # Combat Manager (전투 시뮬레이션 및 집계 엔진)
├── mdc/               # Data Collector/Generator (가상 타임라인 생성 엔진)
├── nexon_api/         # Nexon API Loader (외부 데이터 수집기)
└── shared/            # 공통 자산 (규격 및 데이터)
    ├── proto/         # Protocol Buffers 규격
    │   ├── nexon/     # 넥슨 공식 API 규격
    │   └── internal/  # 엔진 내부 시뮬레이션 규격
    └── data/          # YAML 프리셋 데이터
        ├── classes/   # 직업별 스킬 사양
        ├── mobs/      # 몬스터/보스 정보
        ├── policies/  # 행동 결정 정책
        └── presets/   # 캐릭터 스펙 프리셋
```

## 참고 및 인용 자료 (References)

MCC 연산 엔진의 수식 및 로직은 다음 자료를 기반으로 구현되었습니다.
- [메이플스토리 인벤: 전투력 공식 분석](https://www.inven.co.kr/board/maple/2299/5679951)
- [oleneyl/maplestory_dpm_calc (GitHub)](https://github.com/oleneyl/maplestory_dpm_calc)

## 개발 및 빌드

### 요구 사항
- C++ 20 이상 호환 컴파일러
- [CMake](https://cmake.org/) (3.15 이상)
- [Boost](https://www.boost.org/) (Multiprecision)
- [Emscripten SDK](https://emscripten.org/) (Wasm 빌드 시 필요)
- [Protobuf Compiler (protoc)](https://protobuf.dev/)

### 빌드 명령어 (Makefile)
```bash
# 1. 일반 리눅스 빌드 (CLI용)
make build

# 2. WebAssembly 빌드 (웹 프론트엔드용)
make wasm

# 3. 테스트 실행 (Native)
make test

# 4. 빌드 정리
make clean
```

## 라이선스 (License)

이 프로젝트는 **GNU Affero General Public License v3.0 (AGPL-3.0)** 을 따릅니다.

## AI 어시스턴트

이 프로젝트는 **Gemini CLI**를 사용하여 개발하고 있습니다.
