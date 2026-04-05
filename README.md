# Maple Combat Calculator (MCC)

C++ 엔진과 WebAssembly를 기반으로 구현된 메이플스토리 전투력 및 데미지 계산기 프로젝트입니다.

## 1. 개요
이 프로젝트는 복잡한 데미지 공식과 대량의 전투 로그를 브라우저 환경에서 빠르고 정확하게 처리하기 위해 설계되었습니다. 모든 핵심 연산 로직은 C++로 작성되었으며, Emscripten을 통해 WebAssembly(Wasm)로 컴파일되어 웹 브라우저에서 실행됩니다.

## 2. 핵심 엔진 컴포넌트

엔진은 책임 범위에 따라 세 가지 주요 모듈로 나뉩니다.

### 2.1. MDC (Maple Data Collector/Converter)
- **역할**: 외부 데이터 및 유저 입력을 엔진 내부 엔티티(Entity)로 변환하고, 시뮬레이션에 필요한 레코드(Record)를 생성하는 게이트웨이입니다.
- **주요 기능**:
  - 넥슨 Open API 응답(Protobuf) 및 사용자 사양을 엔진 내부 구조로 변환.
  - 시뮬레이션용 레코드(Record/JSON) 데이터 생성 및 로드.
  - YAML 기반의 보스/몬스터 데이터 및 스킬 프리셋 관리.
- **진행 상황**: 
  - YAML 기반 캐릭터/몬스터/스킬/정책 로더 구현 완료.
  - 시뮬레이션 엔진(`mdc::Engine`) 및 엔티티(`mdc::Entity`) 시스템 구축 완료.
  - 시뮬레이션 후 `combat_log.bin` 생성 기능 구현됨.
- **TODO**:
  - `BattlePracticeCharacterInfo` 프로토콜 버퍼 데이터를 캐릭터 엔티티로 자동 변환하는 파서 구현.
  - 공식 API의 상세 장비/스탯 데이터를 기반으로 한 정밀 입력 모듈 개발.

### 2.2. MCM (Maple Combat Manager/Module)
- **역할**: 캐릭터 시뮬레이션을 수행하고, MDC에서 생성된 레코드를 읽어 전투 컨텍스트를 제어합니다.
- **주요 기능**:
  - 캐릭터 동작 시뮬레이션 및 데이터 흐름 제어.
  - `Aggregator`를 통한 캐릭터/장비/스킬 스탯의 단계별 집계.
  - MCC를 호출하여 최종 계산을 수행하는 오케스트레이터 역할.
- **진행 상황**:
  - `combat_log.bin` 로드 및 실행(Replay) 기본 구조 구현 완료.
  - `Aggregator`를 통한 데이터 집계 로직 구축 중.
  - `print_report()`를 통한 기초적인 결과 출력 지원.
- **TODO**:
  - `SkillTimeline` 데이터를 소비하여 실제 인게임 전투 기록을 그대로 재현(Replay)하는 엔진 확장.
  - 타임라인 기반의 버프 가동률 및 딜 로스(Loss) 분석 기능 추가.

### 2.3. MCC (Maple Combat Calculator)
- **역할**: 주어진 정보를 바탕으로 실제 전투력(Combat Power) 및 스킬 기대 데미지를 산출하는 연산 엔진입니다.
- **주요 기능**:
  - 캐릭터의 종합 전투력 계산.
  - 특정 조건(방어율, 레벨 차이 등)에 따른 스킬 기대 데미지 도출.
- **진행 상황**:
  - 전투력(`calculateCombatPower`) 및 숙련도/맥뎀 보정이 반영된 스킬 데미지(`applyMaxDamageCorrection`) 계산 로직 구현 완료.
- **제약 사항 (현재 미지원)**:
  - 데몬어벤져 (HP 기반 주스탯)
  - 제논 (다중 주스탯)
- **TODO**:
  - `BattlePracticeResult`를 정답셋(Ground Truth)으로 활용하여 계산 공식의 오차 자동 검증 및 보정(Calibration).
  - 공식 API의 실제 누적 데미지와 엔진 계산값 간의 비교 분석 모듈 구현.

## 3. 데이터 규격 (Protobuf)

시스템 간의 데이터 교환 및 엔진 내부 데이터 구조의 일관성을 위해 **Google Protocol Buffers**를 사용합니다.

- 정의 위치: `shared/proto/`
- 주요 메시지:
  - `battle_practice.proto`: 넥슨 Open API(연무장) 응답 규격
  - `battle_practice_result.proto`: 연무장 전투 결과 데이터
  - `skill_timeline.proto`: 스킬 사용 기록 타임라인

## 4. 프로젝트 구조

```text
/
├── mcc/               # Core Calculator (전투력/스킬 데미지 엔진)
├── mcm/               # Combat Manager (데이터 집계 및 컨텍스트 관리)
├── mdc/               # Data Collector (엔진 엔트리포인트 및 엔티티 로더)
├── data/              # 스킬 데이터 및 보스 프리셋 (YAML)
└── shared/            # 공용 헤더 및 유틸리티
```

## 5. 참고 및 인용 자료 (References)
MCC 연산 엔진의 수식 및 로직은 다음 자료를 기반으로 구현되었습니다.
- [메이플스토리 인벤: 전투력 공식 분석](https://www.inven.co.kr/board/maple/2299/5679951)
- [oleneyl/maplestory_dpm_calc (GitHub)](https://github.com/oleneyl/maplestory_dpm_calc)

## 6. 개발 및 빌드

### 요구 사항
- C++ 17 이상 호환 컴파일러
- [CMake](https://cmake.org/) (3.10 이상)
- [Emscripten SDK](https://emscripten.org/) (Wasm 빌드 시 필요)
- [Protobuf Compiler (protoc)](https://protobuf.dev/)

### 로컬 빌드 (C++ 테스트용)
```bash
mkdir build && cd build
cmake ..
make
```

## 7. 라이선스 (License)

이 프로젝트는 **GNU Affero General Public License v3.0 (AGPL-3.0)**을 따릅니다.

상세한 내용은 [LICENSE](LICENSE) 파일을 참고하시기 바랍니다.
