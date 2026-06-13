# MathSolver — 프로젝트 설계 명세서 v2
> Cursor AI에게 이 문서를 통째로 넘기고 "Phase 1부터 step by step으로 구현 가이드해줘" 라고 하면 됩니다.

---

## 1. 프로젝트 개요

### 한 줄 정의
이산수학 · 선형대수 특화 **C++ 공학계산기 + LLM 복합문제 풀이 엔진**

### 핵심 목적
- 도구 완성도 우선, Cursor AI와 함께 개발하며 C++ 학습 병행
- 단순 계산은 빠르고 간결하게, 복합 문제는 단계별 풀이 과정과 함께 출력
- 공학용 계산기(TI-Nspire CX II CAS)를 레퍼런스로 삼되, **풀이 과정 출력**이 핵심 차별점

### 우선순위
1. 풀이 과정 출력 (최우선)
2. 정확도
3. 확장성 (문제 유형 추가)
4. 처리 속도

---

## 2. 기술 스택

| 항목 | 선택 | 이유 |
|---|---|---|
| 언어 | C++17 이상 | 성능, 구조 설계 명확성, FTXUI 지원 |
| 빌드 | CMake | 크로스플랫폼, 라이브러리 관리 |
| TUI | FTXUI | C++ 헤더온리, 크로스플랫폼 터미널 UI |
| 테스트 | Google Test 또는 Catch2 | 단위 테스트 |
| JSON | nlohmann/json | 헤더온리, LLM API 응답 파싱 |
| HTTP | cpp-httplib | 헤더온리, LLM API 호출 |
| LLM | Claude API 또는 OpenAI API | 복합 문제 풀이 + Vision OCR (Phase 5) |

---

## 3. 전체 아키텍처

```
MathSolver (진입점)
├── calc 모드     → 즉시 수치 반환, 간결한 출력
└── LLM 경로      → Pass 1 분해 → C++ 계산 → Pass 2 최종 풀이 조립

핵심 레이어 구조:
[InputAdapter]        ← 입력 소스 정규화 (Phase 1: 키보드 / Phase 5: 파일·이미지 추가)
      ↓
[InteractiveSession]  ← FTXUI 기반 대화형 UI + SessionContext 보유
      ↓
[ProblemParser]       ← 문제 유형 태깅, 단순/복합 판별 (LLM·계산 없음)
      ↓
[SolveRouter]         ← 경로 결정 및 LLM 호출 위임
  ├── 단순 → C++ 엔진 직접 처리
  └── 복합 → LLM 2-pass 파이프라인
      ↓
[Solver Interface]    ← 추상 클래스, 각 유형별 구현체 상속
  ├── LinearAlgebraSolver
  └── DiscreteMathSolver
      ↓
[CoreMath]            ← 실제 알고리즘 구현, 모든 Solver가 공유
      ↓
[SolverResult]        ← answer + steps: vector<SolutionStep>
      ↓
[OutputFormatter]     ← verbose / compact 출력, FTXUI 렌더링
```

---

## 4. 핵심 데이터 구조

```cpp
// 풀이 한 단계를 표현
struct SolutionStep {
    std::string description;   // "2행에서 1행×3을 빼기"
    std::string math_expr;     // "R2 = R2 - 3·R1"
    std::string result_state;  // 현재 행렬 상태 스냅샷 (문자열)
    StepType type;             // ELIMINATION / SUBSTITUTION / THEOREM_APPLY / ...
};

// 모든 연산의 반환 타입
struct SolverResult {
    std::variant<double, Matrix<double>, std::string> answer;
    std::vector<SolutionStep> steps;  // 항상 생성, 출력은 선택
    bool verified;
    SolveRoute route;  // DIRECT(C++ 엔진) / LLM
};

// 행렬 (템플릿으로 설계)
template<typename T>
class Matrix {
    std::vector<std::vector<T>> data;
    size_t rows, cols;
public:
    // 연산자 오버로딩: +, -, *, ==
    // 주요 메서드: transpose(), det(), inverse(), rref() ...
};

// 입력 정규화 구조체
struct ProblemText {
    std::string content;        // 정규화된 문제 텍스트
    InputSource source;         // KEYBOARD / FILE / IMAGE
};

// 문제 파서 결과
struct ParsedProblem {
    ProblemCategory category;   // MATRIX / LOGIC / BASE / MIXED
    ComplexityLevel complexity; // SIMPLE / COMPLEX
    std::string normalized_text;
};

// 대화 메시지 한 턴
struct ChatMessage {
    std::string role;     // "user" / "assistant"
    std::string content;
};

// 세션 컨텍스트 (calc 결과 재사용 + 풀이 후 대화)
struct SessionContext {
    std::map<std::string, SolverResult> variables; // "ans", "A", "result_1" ...
    std::vector<SessionEntry> history;              // 이전 계산 기록
    std::optional<SolverResult> last_result;        // 마지막 풀이 결과 (대화 컨텍스트용)
    std::vector<ChatMessage> chat_history;          // 풀이 후 대화 히스토리
};
```

---

## 5. 모드 구조

### calc 모드 (공학계산기)
- 목적: 빠른 수치 계산
- 결과는 자동으로 세션 변수(`ans`, `result_1`, `result_2` ...)에 저장
- 이전 결과를 다음 계산 입력으로 재사용 가능
- 에러 발생 시 에러 메시지에 원인 설명 포함 (예: `det = 0 → 역행렬 불가`)
- `help` 명령어 입력 시 현재 에러 컨텍스트를 LLM에 넘겨 설명 출력

```
[연산 선택 메뉴]
calc matrix
 ├── 사칙연산 (덧셈·뺄셈·곱셈)
 ├── 역행렬
 ├── 행렬식 (det)
 ├── 전치행렬
 ├── RREF
 ├── LU 분해
 ├── 고유값·고유벡터
 └── 연립방정식 (Ax=b)

calc base
 ├── 진법 변환 (2~36진법, 정수·소수점, 0b/0h prefix 지원)
 ├── 모듈러 산술
 └── GCD · LCM

calc logic
 ├── 진리표 생성
 ├── 항진·모순 판별
 ├── CNF · DNF 변환
 ├── 논리 동치 검사
 └── 집합 연산
```

### 풀이 후 대화 모드
- 풀이가 끝난 뒤 자유롭게 질문 가능 — 별도 모드 전환 없음
- `SessionContext.last_result`와 `chat_history`를 LLM에 넘겨 대화 맥락 유지
- `ProblemParser`가 입력을 판단해 경로 자동 분기:
  - 질문 → LLM 대화 경로 (chat_history에 누적)
  - 계산 요청 → 기존 calc / LLM 2-pass 경로

```
결과 A⁻¹ → result_2 저장됨

▶ 질문이 있으면 입력하거나 새 연산은 메뉴에서 선택
> 3단계에서 왜 5를 곱한 거야?           ← 질문 → LLM 대화 경로

  1행의 피벗이 1이고 3행 1열이 5이므로
  R3 - 5·R1 을 하면 3행 1열이 0이 됩니다.
  이것이 가우스 소거의 핵심입니다.

> 이 역행렬로 Ax=b 풀어줘              ← 계산 요청 → calc 경로로 자동 전환
```

### LLM 2-pass 파이프라인 (복합 문제)
- Pass 1: LLM이 문제를 분석해 필요한 계산 목록 반환 (수치 계산 X)
- C++ 엔진이 목록대로 중간값 전량 계산
- Pass 2: 중간값을 프롬프트에 주입 → LLM이 최종 풀이 서술 조립
- LLM은 설명만 담당, 수치 계산은 항상 C++ 엔진

```json
// Pass 1 LLM 반환 예시
{
  "problem_type": "eigenvalue_diagonalization",
  "required_computations": [
    { "op": "det",         "target": "A - λI" },
    { "op": "eigenvalues", "target": "A" },
    { "op": "eigenvectors","target": "A" },
    { "op": "diagonalize", "target": "A" }
  ],
  "explanation_needed": true
}
```

---

## 6. UX 설계

### 세션 변수 재사용 흐름
```
> calc matrix → 역행렬 → 행렬 입력
  결과 A⁻¹ → 자동 저장: ans, result_1

> calc matrix → 사칙연산 → ans * B
  이전 결과를 변수처럼 사용 가능

> help
  [현재 에러 컨텍스트를 LLM에 전달 → 설명 출력]
```

### 대화형 행렬 입력 흐름
```
> 행 수: 3
> 열 수: 3
> 1행 입력 (공백 구분): 1 2 3
> 2행 입력 (공백 구분): 0 1 4
> 3행 입력 (공백 구분): 5 6 0

입력된 행렬:
⎡  1  2  3 ⎤
⎢  0  1  4 ⎥
⎣  5  6  0 ⎦

계산하시겠습니까? (y/n): y

결과 A⁻¹:                         → result_1로 자동 저장
⎡ -24  18   5 ⎤
⎢  20 -15  -4 ⎥
⎣  -5   4   1 ⎦

풀이 과정 보기? (y/n): y

[단계 1] 첨가행렬 구성 [A|I]
[단계 2] R3 = R3 - 5·R1
...
```

### 입력 검증 원칙
- 각 단계에서 즉시 검증, 해당 행만 재입력 (전체 재입력 X)
- 에러 메시지에 원인 설명 포함 (`det = 0 → 역행렬 불가. help로 자세한 설명 확인`)

### FTXUI 레이아웃
```
┌─────────────────────────────────────────┐
│  MathSolver — 행렬 계산기               │  ← 타이틀바
├──────────────┬──────────────────────────┤
│ 연산 선택    │ 입력 행렬 A   3×3        │  ← 메인 패널
│              │ ⎡  1  2  3 ⎤             │
│ ▶ 역행렬     │ ⎢  0  1  4 ⎥             │
│   행렬식     │ ⎣  5  6  0 ⎦             │
│   전치행렬   ├──────────────────────────┤
│   RREF       │ 풀이 과정  [STEPS ON]    │
│   LU 분해    │ [1] 첨가행렬 구성        │
│              │ [2] R3 = R3 - 5·R1       │
│              │ ...                      │
│              ├──────────────────────────┤
│              │ ▶ 질문 입력 or 메뉴 선택 │  ← 풀이 후 대화 가능
├──────────────┴──────────────────────────┤
│ ↑↓ 이동  Enter 선택  s steps토글  help  q│  ← 푸터
└─────────────────────────────────────────┘
```

---

## 7. LLM 연동 전략 (Phase 5)

### 핵심 원칙
LLM에게 수치 계산을 맡기지 않는다.
C++ 엔진이 계산한 중간값을 프롬프트에 주입 → LLM은 설명과 풀이 서술만 담당

### Pass 1 프롬프트 구조 (문제 분해)
```
[시스템] 너는 수학 문제 분석 전문가다.
         문제를 분석해서 필요한 계산 목록만 JSON으로 반환해라.
         절대 직접 계산하지 마라.

[문제] {원본 문제 텍스트}

[요청] 이 문제를 풀기 위해 필요한 계산 목록을 반환해라.
       형식: { "problem_type": "...", "required_computations": [...] }
```

### Pass 2 프롬프트 구조 (최종 풀이 조립)
```
[시스템] 너는 수학 풀이 단계 설명 전문가다.
         각 단계를 반드시 "STEP|설명|수식|근거" 형식으로만 출력해라.

[문제] {원본 문제 텍스트}

[C++ 엔진 계산 결과]
- 행렬식: {det_value}
- 고유값: {eigenvalues}
- rank: {rank}
- ... (Pass 1에서 요청한 모든 계산 결과)

[요청] 위 수치를 활용해 풀이를 단계별로 설명해라.
```

### 풀이 후 대화 프롬프트
```
[시스템] 너는 수학 풀이 설명 전문가다.
         사용자가 방금 완료된 풀이에 대해 질문하고 있다.
         줄글로 친절하게 설명해라.

[방금 풀이 결과]
- 연산: {operation}
- 입력: {input_summary}
- 결과: {result_summary}
- 풀이 단계: {steps_summary}

[대화 히스토리]
{chat_history}

[사용자 질문] {user_question}
```

### help 명령어 프롬프트
```
[시스템] 너는 수학 개념 설명 전문가다.

[상황] 사용자가 다음 연산을 시도했으나 에러가 발생했다:
- 연산: {operation}
- 에러: {error_message}
- 입력값: {input_summary}

[요청] 에러 원인을 수학적으로 설명하고, 해결 방법을 제안해라.
```

### 복합 문제 판별 기준 (ProblemParser)
- 증명 문제 → LLM 경로
- 서술형 해석 필요 → LLM 경로
- 단순 수치 계산 → C++ 직접 처리
- 중간 복잡도 (대각화, 직교화 등) → C++ 계산 후 LLM 설명

### 입력 소스별 처리 (InputAdapter)
| Phase | 지원 입력 | 처리 방식 |
|---|---|---|
| Phase 1~4 | 키보드 | 직접 텍스트 입력 |
| Phase 5 | 텍스트 파일 | 파일 읽기 → ProblemText |
| Phase 5 | 이미지 (PNG/JPG) | LLM Vision API → 수식 OCR → ProblemText |

---

## 8. 디렉토리 구조 (목표)

```
MathSolver/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── input/                 ← 입력 레이어
│   │   ├── InputAdapter.h / .cpp
│   │   ├── KeyboardInput.h / .cpp
│   │   └── FileInput.h / .cpp       (Phase 5)
│   ├── core/                  ← CoreMath (알고리즘)
│   │   ├── Matrix.h / .cpp
│   │   ├── LinearAlgebra.h / .cpp
│   │   ├── BaseConvert.h / .cpp
│   │   └── Logic.h / .cpp
│   ├── solver/                ← Solver 계층
│   │   ├── Solver.h           (추상 인터페이스)
│   │   ├── LinearAlgebraSolver.h / .cpp
│   │   ├── DiscreteMathSolver.h / .cpp
│   │   ├── ProblemParser.h / .cpp   ← 유형 태깅·판별만
│   │   └── SolveRouter.h / .cpp     ← 경로 결정·LLM 위임
│   ├── session/               ← 세션 관리
│   │   └── SessionContext.h / .cpp
│   ├── ui/                    ← FTXUI 인터페이스
│   │   ├── InteractiveSession.h / .cpp
│   │   ├── OutputFormatter.h / .cpp
│   │   └── MatrixWidget.h / .cpp
│   └── llm/                   ← LLM 연동 (Phase 5)
│       ├── LLMConnector.h / .cpp
│       └── PromptBuilder.h / .cpp
└── tests/
    ├── test_matrix.cpp
    ├── test_linear_algebra.cpp
    ├── test_base_convert.cpp
    └── test_logic.cpp
```

---

## 9. 개발 로드맵

### Phase 1 — 프로젝트 골격 + 행렬 기초
**목표**: calc matrix 기본 연산 동작 확인

- [ ] CMake 프로젝트 셋업
- [ ] InputAdapter 인터페이스 확정 (KeyboardInput 구현체만)
- [ ] Matrix 클래스 (템플릿, 연산자 오버로딩)
- [ ] CoreMath: 사칙연산, 전치, det, 역행렬
- [ ] OutputFormatter: 행렬 박스 렌더링, ANSI 색상
- [ ] 단위 테스트 구조 셋업 (Catch2 또는 GTest)

**C++ 학습 포인트**: 클래스 설계, 템플릿 기초, 연산자 오버로딩, 예외 처리

---

### Phase 2 — 행렬 심화 + SolutionStep 도입
**목표**: steps ON/OFF 풀이 과정 출력 동작 확인

- [ ] CoreMath: 가우스 소거, RREF, LU 분해, rank/nullity, Ax=b
- [ ] SolutionStep / SolverResult 구조체 확정
- [ ] 각 알고리즘에 step 기록 로직 추가
- [ ] CoreMath: 진법 변환 (0b/0h prefix), 모듈러 산술, GCD/LCM
- [ ] CoreMath: 진리표, CNF/DNF, 논리 동치
- [ ] Solver 추상 클래스 + LinearAlgebraSolver / DiscreteMathSolver
- [ ] ProblemParser 초안 (유형 태깅·단순/복합 판별)
- [ ] SolveRouter 초안 (C++ 직접 경로만)

**C++ 학습 포인트**: 상속, 가상 함수, 다형성, std::variant, std::vector 심화

---

### Phase 3 — FTXUI 인터페이스 + SessionContext 연동
**목표**: calc 모드 완성 — 공학계산기로서 단독 사용 가능

- [ ] CMake에 FTXUI 의존성 추가
- [ ] 메뉴 + 메인 패널 레이아웃
- [ ] 키보드 네비게이션 (↑↓ Enter)
- [ ] 행렬 대화형 입력 위젯
- [ ] 결과 + steps 패널 렌더링
- [ ] s키 steps 토글, r키 재입력, q키 종료
- [ ] 진법·논리 입력 흐름 연동
- [ ] OutputFormatter 완성 (verbose / compact)
- [ ] SessionContext 구현 (세션 변수 저장·재사용)
- [ ] 에러 메시지에 원인 설명 포함

**C++ 학습 포인트**: 외부 라이브러리 통합, 이벤트 루프, 콜백 패턴

---

### Phase 4 — 고급 알고리즘 + ProblemParser 완성
**목표**: LLM 없이 단순~중간 복잡도 문제 자력 풀이 가능

- [ ] CoreMath: 고유값·고유벡터 (QR 알고리즘)
- [ ] CoreMath: 대각화 판별·수행
- [ ] CoreMath: Gram-Schmidt 직교화
- [ ] ProblemParser 완성 (복합 문제 판별 고도화)
- [ ] SolveRouter 완성 (복합 경로 체이닝)
- [ ] 단계별 Solver 체이닝

**C++ 학습 포인트**: 수치 알고리즘, 복잡한 상태 관리, 체이닝 패턴

---

### Phase 5 — LLM 연동 + 입력 확장 + 마무리
**목표**: 전체 시스템 완성

- [ ] cpp-httplib 연동 + nlohmann/json 파싱
- [ ] LLMConnector 구현
- [ ] PromptBuilder: Pass 1 (문제 분해) / Pass 2 (풀이 조립) / help / 풀이 후 대화
- [ ] LLM 응답 파싱 → SolutionStep 변환
- [ ] 응답 검증 레이어 + 폴백 처리
- [ ] SessionContext: chat_history 누적 · last_result 저장
- [ ] ProblemParser: 질문 vs 계산 요청 판별 로직 추가
- [ ] InputAdapter: FileInput 구현체 추가 (텍스트 파일)
- [ ] InputAdapter: ImageInput 구현체 추가 (LLM Vision OCR)
- [ ] 비용 최소화 프롬프트 튜닝
- [ ] 전체 통합 테스트

**C++ 학습 포인트**: 네트워크 프로그래밍, JSON 처리, 비동기 처리 기초

---

## 10. Cursor 활용 팁

이 문서를 Cursor에 넘길 때 이렇게 요청하세요:

```
이 문서가 내가 만들 C++ 프로젝트의 전체 설계야.
Phase 1부터 시작해서 각 항목을 하나씩 구현할 수 있도록
step by step으로 가이드해줘.
각 단계마다:
1. 무엇을 만드는지 설명
2. 실제 코드 작성
3. 확인 방법 (빌드·테스트)
순서로 진행해줘.
```
