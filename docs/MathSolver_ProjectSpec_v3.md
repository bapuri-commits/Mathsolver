# MathSolver — 프로젝트 설계 명세서 v3
> 이 문서는 v1·v2를 대체하는 **현행 설계**입니다.
> v1·v2의 종착지(LLM 위임 풀이 엔진)는 유지하되, **밑에서부터 쌓는 순서**와 **분수 토대**를 새 원칙으로 다시 세웠습니다.
> 새 작업(예: Fraction 구현)을 시작할 때 이 문서를 출발점으로 삼습니다.

---

## 0. 이 문서가 v2와 달라진 점 (요약)

| 축 | v1 / v2 | v3 (현행) |
|---|---|---|
| 수치 타입 | `Matrix<double>` | **`Matrix<Fraction>` (유리수 기반)** |
| 1순위 토대 | 명시 안 됨 | **Fraction(분수) 클래스가 모든 것의 바닥** |
| 계산 ↔ 출력 | StepPrinter에서 결합 (계산하며 `os <<`) | **완전 분리 — 계산은 step을 쌓아 반환, 출력은 별도** |
| 개발 순서 | calc/solve 모드 + LLM을 비교적 이른 단계에 배치 | **연산 backend 완성이 먼저, LLM(머리)은 맨 마지막** |
| LLM 연동 방식 | API 2-pass 우선 | **공부용은 MCP 우선, 제품화 단계에서 API** |
| Phase 수 | 5 | 4 + 보류(5) |
| 기존 Cursor 코드 | — | **읽고·이해하고·고치는 것을 학습 방식으로 채택** |

핵심 통찰: **"선형대수 풀이 엔진"이 최종 목표가 맞지만, 그 엔진의 가치는 "정확한 손"(연산 backend)에서 나온다.** LLM은 머리, 이 모듈은 손. 손이 부정확하면(=double로 0.333을 뱉으면) 위임의 의미가 없다. 그래서 분수가 1순위다.

---

## 1. 프로젝트 개요

### 한 줄 정의
이산수학 · 선형대수 특화 **C++ 공학계산기 + (장기) LLM 복합문제 풀이 엔진**

### 진짜 목표 (단계별)
1. **지금**: 입력한 행렬 연산을, 손으로 푸는 순서 그대로, **분수로 정확하게**, 보기 좋은 단계별 해설로 출력하는 도구.
2. **장기**: 위 도구를 "연산 backend"로 삼아, LLM이 문제를 해석·설계하고 실제 수학 연산은 이 모듈에 위임하는 풀이 엔진.

### 핵심 차별점
**"정답"이 아니라 "풀이 과정"이 결과물이다.** 일반 공학용 계산기는 답만 준다. 이 도구는 해설지를 준다.

### 우선순위 (확정)
1. **분수 정확도** (최우선 — 모든 토대)
2. **예쁜 해설 출력** (보기 좋은 단계별 풀이)
3. C++ 학습 (만드는 과정에서 자연히 따라옴 — 단, 동기로서는 사실상 상위)
4. 확장성 (문제 유형 추가)
5. 처리 속도

---

## 2. 핵심 설계 원칙 (절대 규칙)

### 원칙 1 — 분수가 바닥이다
모든 계산의 밑바닥에 `Fraction`(유리수) 클래스가 있다. `Matrix<double>`이 아니라 `Matrix<Fraction>`으로 간다.
- double로 시작하면 분수는 영영 불가능하다 (0.333 → 1/3 복원 불가).
- 분수여야 가우스 소거·역행렬의 **모든 중간값이 정확**하게 남는다 (`R2 = R2 - (3/5)·R1`).
- 이것이 "문제집 해설" 느낌의 정체이자, LLM이 신뢰할 수 있는 ground truth의 조건.

### 원칙 2 — 계산과 출력을 절대 섞지 않는다
알고리즘은 **데이터(step)만 쌓아 반환**한다. 화면 출력은 그 데이터를 받아 **별도 함수**가 처리한다.

```cpp
// ❌ 이렇게 하지 않는다 (현재 StepPrinter 방식)
void gaussElimination(Matrix<Fraction>& m) {
    // ... 계산 ...
    std::cout << "R2 = R2 - " << factor << "·R1\n";  // 출력이 알고리즘에 박힘
}

// ✅ 이렇게 한다
SolverResult gaussElimination(Matrix<Fraction> m) {
    std::vector<SolutionStep> steps;
    // ... 계산하면서 ...
    steps.push_back({"2행에서 1행의 3/5배를 빼기",
                     "R2 = R2 - (3/5)·R1",
                     m.toString()});
    return { answer, steps };   // 출력은 전혀 하지 않음
}

// 출력은 완전히 별개. 같은 steps를 받아서:
renderForHuman(result.steps);   // 지금 만들 것 — 예쁜 박스
renderForLLM(result.steps);     // 나중에 추가 — JSON. 알고리즘은 한 줄도 안 바뀜
```

이 분리가 지켜지면, 5단계에서 LLM용 출력을 추가할 때 알고리즘 코드는 **한 줄도 바뀌지 않는다.** `renderForLLM` 함수 하나 새로 쓰면 끝.

### 원칙 3 — 손을 먼저, 머리는 나중에
LLM(머리)은 연산 모듈(손)이 완벽해진 뒤에 붙인다. 손이 부정확한 상태에서 머리부터 붙이면, LLM이 잘못 설계한 건지 모듈이 잘못 계산한 건지 디버깅이 불가능해진다.

### 원칙 4 — 각 연산은 독립 호출 가능한 단위다
모든 연산은 `SolverResult`를 깨끗이 반환하는 독립 함수로 만든다. 이래야 나중에 MCP 도구로 감싸든 API 응답에 넣든 자유롭다. (원칙 2와 같은 뿌리.)

---

## 3. 기술 스택

| 항목 | 선택 | 이유 |
|---|---|---|
| 언어 | C++17 이상 | 메모리·값·자원의 이해 = C++ 학습의 핵심. OOP/패턴은 자바와 동급으로 다 들어감 |
| 빌드 | CMake | 크로스플랫폼, 라이브러리 관리 (현재 구성 유지) |
| 개발환경 | Cursor / MSVC / VS Code 병행 | — |
| 테스트 | Catch2 (현재 도입됨) | 단위 테스트. Fraction부터 테스트 우선 |
| TUI | FTXUI (장기) | 출력 "예쁘게"의 상한선. 단, 우선은 CLI로 충분 |
| JSON | nlohmann/json (Phase 5) | LLM 연동 시 |
| HTTP | cpp-httplib (제품화 단계) | API 직접 호출 시 |
| LLM | **공부용: MCP / 제품용: Claude API** | 아래 7장 참고 |

> **수치 타입 결정**: 분자·분모를 `long long`으로 둔다. 오버플로는 직접 막아야 하며(약분을 항상 수행해 완화), 학습 목적상 이 고생 자체가 C++ 학습 내용이다. (자바였다면 `BigFraction`으로 회피 가능했겠지만, C++ 학습이 목표이므로 직접 구현을 택함.)

---

## 4. 현재 코드 상태 분석 (출발점)

> 기존 레포(`bapuri-commits/Mathsolver`)는 **행렬 클래스를 만든 뒤 Cursor에 "맛보기"로 주문해 나온 레퍼런스 샘플**이다. 내가 설계한 것이 아니므로, 취할 것만 취하고 고칠 것은 고치고 버릴 것은 버린다. 약 1,400줄, C++ 98.7%.

### 이미 잘 되어 있는 것 (자산)
- **`Matrix<T>`가 진짜 템플릿이다.** double 고정이 아니라 `T`로 일반화됨 → Fraction 갈아끼우기의 토대.
- 연산자 오버로딩(`+ - * == !=`), 정적 팩토리(`identity`), `initializer_list` 생성자 완비.
- 알고리즘 풍부: det(여인수 전개), 역행렬(수반행렬), RREF, LU/PLU/LDU, pow(분할정복), trace, adjugate.
- REPL + 재귀 하향 식 파서(`A + B * C`를 우선순위까지 지켜 계산). v2의 SessionContext가 `std::map<string,Matrix>`로 사실상 구현됨.
- `-v` 플래그로 det/inverse/rref/lu/plu/ldu의 단계별 출력 — "해설" 방향을 이미 시작.

### 우리 설계와 충돌하는 두 가지 (= 처음 불만의 근본 원인)
- **충돌 1: 전부 `double`.** `main`이 `Matrix<double>`, `StepPrinter`는 `using Mat = Matrix<double>`로 못 박음. 곳곳에 `std::abs(x) < 1e-12` 부동소수점 땜질, `formatValue`는 소수 4자리 반올림 → **0.333을 만들도록 설계됨, 1/3 복원 불가.**
- **충돌 2: 계산과 출력이 한 몸.** `StepPrinter`는 계산하며 `os <<`로 즉시 출력. 게다가 일반 알고리즘(`rref()`)과 해설용(`rrefVerbose()`)이 **같은 로직을 두 번 구현.** → LLM에 줄 구조화 데이터를 뽑을 수 없음.

### 파일별 운명 (확정)
| 파일 | 운명 | 이유 |
|---|---|---|
| `Matrix.hpp` | **고쳐서 살린다** | 최고 자산. 템플릿·연산자·알고리즘 로직 유지. `std::abs`·`1e-12`·`formatValue`의 소수 처리만 들어내고 `Fraction` 대응. 고치는 과정이 곧 템플릿 학습. |
| `StepPrinter.hpp` | **로직(아이디어)만 취하고 버린다** | 출력 결합 구조는 폐기. 단 "단계를 어떻게 나누는가"의 흐름은 새 `SolutionStep` 기반으로 다시 쓸 때 참고. |
| `CommandParser.hpp` | **보류 → 나중에 고친다** | REPL·파서는 좋으나 `Matrix<double>`+출력 결합에 묶임. 토대·알고리즘 정착 후 Phase 3에서 Fraction 기반으로 재연결. |
| `SquareMatrix.hpp` | **버린다** | 거의 빈 껍데기. `Matrix<T>`가 정사각 연산을 이미 처리. 필요해지면 그때 재생성. |
| `CMakeLists.txt` | **유지** | Catch2 포함 빌드 구성 그대로 활용. |

---

## 5. 핵심 데이터 구조 (목표)

```cpp
// 모든 것의 바닥 — 유리수
class Fraction {
    long long num;   // 분자
    long long den;   // 분모 (항상 양수로 정규화; 음수는 num이 가짐)
public:
    // 생성 시 gcd로 자동 약분
    // 연산자 오버로딩: + - * / == != (< 등 비교도)
    // toString(): 정수면 "3", 분수면 "5/7", 음수면 "-2/3"
};

// 행렬 (이미 템플릿 — 그대로 살림)
template<typename T>
class Matrix {
    std::vector<std::vector<T>> data_;
    size_t row_, col_;
    // 연산자 오버로딩, transpose/det/inverse/rref/lu... (Fraction 대응으로 수정)
};

// 풀이 한 단계 (계산/출력 분리의 핵심 그릇)
struct SolutionStep {
    std::string description;   // "2행에서 1행의 3/5배를 빼기"
    std::string math_expr;     // "R2 = R2 - (3/5)·R1"
    std::string result_state;  // 현재 행렬 상태 스냅샷
    StepType type;             // ELIMINATION / SUBSTITUTION / THEOREM_APPLY / ...
};

// 모든 연산의 반환 타입
struct SolverResult {
    std::variant<Fraction, Matrix<Fraction>, std::string> answer;
    std::vector<SolutionStep> steps;  // 항상 생성, 출력은 선택
    bool verified;
};
```

> v2에 있던 `ProblemText`, `ParsedProblem`, `SessionContext`, `InputSource` 등은 **Phase 3~5로 미룬다.** 지금 데이터 구조의 핵심은 `Fraction` → `Matrix<Fraction>` → `SolutionStep` → `SolverResult` 네 가지뿐.

---

## 6. 개발 로드맵 (4단계 + 보류)

### Phase 1 — 분수 엔진 + 행렬 뼈대
**목표**: `1/2 + 1/3 = 5/6`이 콘솔에 찍힌다. 분수 사칙연산과 기본 행렬 연산이 정확히 동작.

- [ ] `Fraction` 클래스 단독 구현 (약분·정규화·사칙연산·비교·`toString`)
- [ ] `Fraction` 단위 테스트 (Catch2) — **행렬보다 먼저, 토대부터 검증**
- [ ] `Matrix.hpp`를 `Fraction` 대응으로 수정 (`std::abs`/`1e-12`/소수 포맷 제거)
- [ ] `Matrix<Fraction>`로 덧셈·뺄셈·곱셈·전치 동작 확인
- [ ] (이때 역행렬·RREF 등 심화는 손대지 않음 — Phase 2)

**C++ 학습 포인트**: 연산자 오버로딩, "객체가 생성 시점에 스스로 정규화", 템플릿이 타입에 무관하게 도는 원리, 값 복사 vs 참조

---

### Phase 2 — 해설 데이터 구조 + 핵심 알고리즘
**목표**: 역행렬·RREF 풀이가 **step 리스트로 쌓인다** (아직 화면 출력은 단순해도 됨).

- [ ] `SolutionStep` / `SolverResult` 구조체 확정
- [ ] 기존 `Matrix.hpp`의 알고리즘 로직을 참고해, **계산하며 step을 쌓는 방식**으로 재구성
      (가우스 소거, RREF, 역행렬, det) — **원칙 2 준수, 출력 코드 없음**
- [ ] 일반 버전과 해설 버전의 **로직 이중 구현 제거** (하나의 알고리즘이 step을 쌓고, 출력 여부만 선택)
- [ ] (이산수학 카테고리 — 진법/모듈러/논리 — 는 Phase 4)

**C++ 학습 포인트**: `std::variant`, `std::vector` 심화, 구조체 설계, (상속·가상함수는 Solver 추상화 도입 시)

---

### Phase 3 — 예쁜 출력 + REPL 재연결 (← 2순위 목표 실현)
**목표**: 손으로 푼 해설지와 구분이 안 될 만큼 깔끔한 출력. 세션 변수 재사용 동작.

- [ ] `renderForHuman(steps)` — 분수 세로 정렬, 행렬 박스(⎡⎢⎣), 단계 번호, 색상
- [ ] `CommandParser`를 `Fraction` 기반으로 재연결 (변수 저장·식 파싱 복원)
- [ ] 에러 메시지에 원인 설명 포함 (`det = 0 → 역행렬 불가`)
- [ ] (선택) FTXUI 도입은 여기서 검토 — 단 CLI로 충분하면 미뤄도 됨

**C++ 학습 포인트**: 문자열 정렬·포매팅, (FTXUI 도입 시) 외부 라이브러리 통합·이벤트 루프

---

### Phase 4 — 카테고리 확장 + 고급 알고리즘
**목표**: 행렬 외 문제도 같은 품질의 해설로. LLM 없이 단순~중간 복잡도 자력 풀이.

- [ ] 이산수학: 진법 변환(2~36, 0b/0h), 모듈러·GCD·LCM, 진리표, CNF/DNF·논리 동치
- [ ] 고급 행렬: 고유값·고유벡터(QR), 대각화, Gram-Schmidt 직교화
- [ ] 같은 "계산 + step + 출력 분리" 패턴 재사용
- [ ] (필요 시) `Solver` 추상 클래스로 유형별 구현 정리

**C++ 학습 포인트**: 수치 알고리즘, 다형성/인터페이스 설계, 복잡한 상태 관리

---

### Phase 5 (보류) — LLM "머리" 얹기
**목표**: LLM이 문제를 해석·설계하고, 연산은 이 모듈에 위임하는 풀이 엔진.

> **Phase 4까지가 "LLM 없이도 완벽히 도는 연산 backend"다. 그게 끝나야 머리를 붙인다.**

- [ ] **MCP 우선** (공부용): 이 모듈을 MCP 서버로 노출 → Claude 데스크탑이 UI·대화·프롬프팅 담당, 나는 연산 도구만 제공. UI를 직접 만들 필요 없음.
- [ ] (제품화 단계) API 직접 연동: 2-pass 파이프라인을 내 코드가 주도 (cpp-httplib + nlohmann/json)
- [ ] `renderForLLM(steps)` — step을 JSON 등 구조화 포맷으로 직렬화 (알고리즘 불변)
- [ ] 스캔/OCR, 응답 검증, 폴백 — 가장 마지막

**왜 MCP가 먼저인가**: 공부가 목적인 단계에서 프롬프트 정교화에 빠지는 것은 본말전도. MCP는 "잘 관리되는 곳"(Claude 데스크탑)이 흐름을 맡아주므로 수학 엔진에만 집중 가능. 진짜 제품으로 내가 흐름을 통제해야 할 때 비로소 API로 간다.

---

## 7. LLM 연동 방향 (설계 단계 메모)

| 방식 | 머리(주도권) | 장점 | 언제 |
|---|---|---|---|
| **MCP 플러그인** | Claude 데스크탑 | UI/프롬프팅 불필요, 엔진에만 집중 | **공부용 (먼저)** |
| **자체 API** | 내 프로그램 | 흐름 완전 통제, 2-pass 직접 설계 | 제품화 단계 (나중) |

핵심 구분: **"LLM의 사고 과정에 끼어드는 것"은 불가능**(모델 내부). 가능한 것은 **"사고의 산물을 단계 사이에서 가로채 검증·교정"** — 그것이 2-pass이고, 그것이 우리가 원하는 정확성의 메커니즘. 어느 방식이든 이 모듈이 깨끗한 `SolverResult` 인터페이스만 가지면 양쪽 다 수용 가능.

---

## 8. 학습 방식 (이 프로젝트의 공부 모델)

- **백지에서 다 짜지 않는다.** Cursor가 만든 코드를 **읽고·이해하고·내 원칙(분수·분리)에 맞게 고치는 것**도 핵심 공부다.
- 단, "수정"에는 두 종류가 있음을 구분한다:
  - **가벼운 수정**: `double` → `Fraction` (템플릿 덕에 로직 보존, 진짜 '고치기')
  - **무거운 수정**: 계산/출력 분리 (사실상 재설계 — 부분 수정이 아니라 구조를 다시 잡음)
- 무거운 쪽을 가벼운 쪽처럼 다루지 않는다. `StepPrinter`는 "조금씩 손봐 살리기"가 아니라 "아이디어만 취해 새로".
- C++을 택한 이유와 학습이 일치: `Fraction`을 직접 만들며 약분·오버플로·복사·정규화를 배운다 (자바 `BigFraction`이 숨기는 바로 그 층).

---

## 9. 디렉토리 구조 (목표 — 점진 전환)

```
Mathsolver/
├── CMakeLists.txt          # 유지 (Catch2 포함)
├── src/
│   ├── main.cpp            # Matrix<double> → Matrix<Fraction>로 전환
│   ├── core/
│   │   ├── Fraction.hpp    # ★ Phase 1 신규 — 모든 것의 바닥
│   │   ├── Matrix.hpp      # 기존 고쳐 살림 (Fraction 대응)
│   │   └── (BaseConvert / Logic ...)   # Phase 4
│   ├── solver/
│   │   ├── SolutionStep.hpp / SolverResult.hpp  # Phase 2 신규
│   │   └── (algorithms — step 쌓는 방식)        # Phase 2
│   ├── io/
│   │   ├── renderForHuman.hpp   # Phase 3
│   │   └── renderForLLM.hpp     # Phase 5 (구조화 직렬화)
│   ├── CommandParser.hpp   # 보류 → Phase 3에서 Fraction 재연결
│   └── (StepPrinter.hpp 폐기 / SquareMatrix.hpp 폐기)
└── tests/
    ├── test_fraction.cpp   # ★ Phase 1 — 토대부터 테스트
    └── test_matrix.cpp     # 기존 — Fraction 대응으로 갱신
```

> 구조는 점진 전환한다. 한 번에 갈아엎지 않고, `Fraction` 추가 → `Matrix` 대응 → 알고리즘 재구성 → REPL 재연결 순으로 기존 위에서 옮겨간다.

---

## 10. 다음 행동

1. **이 문서를 새 대화창의 출발점으로 삼는다.**
2. **Phase 1, `Fraction` 클래스부터 시작한다.** (행렬·해설·LLM은 일절 손대지 않음)
3. `1/2 + 1/3 = 5/6`과 음수·정수 정규화, 약분이 단위 테스트로 통과하면 Phase 1의 첫 관문 통과.
