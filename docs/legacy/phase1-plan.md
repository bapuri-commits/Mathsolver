# Phase 1 — 실행 계획

> 설계 문서의 7-레이어 아키텍처를 한 번에 만들지 않는다.
> **작은 것 하나 → 실행 확인 → 확장** 사이클로 점진 구현.

## 원칙

- 매 스텝마다 빌드+실행 가능한 상태 유지
- main.cpp에서 직접 테스트하다가, 충분히 쌓이면 레이어로 분리
- InputAdapter, Solver, FTXUI 같은 상위 레이어는 나중에 감싸기

## 스텝 순서

### Step 1 — Matrix 클래스 + 콘솔 출력
- `src/core/Matrix.h` 작성 (템플릿 클래스)
- 생성자, 행렬 출력 (⎡⎢⎣ 박스 렌더링)
- main.cpp에서 행렬 만들어 화면에 찍기
- **확인**: 빌드 → 실행 → 행렬 보임

### Step 2 — 행렬 사칙연산
- 덧셈, 뺄셈, 곱셈 연산자 오버로딩
- 전치행렬
- main.cpp에서 A + B, A * B 등 테스트
- **확인**: 연산 결과 행렬이 올바른지 눈으로 비교

### Step 3 — det + 역행렬
- 행렬식(det) — cofactor 재귀
- 역행렬 — 가우스-조르단
- 예외 처리 (det=0 → 역행렬 불가)
- **확인**: 알려진 행렬로 det, inverse 검증

### Step 4 — 단위 테스트 도입
- Catch2 또는 Google Test 추가
- Step 1~3의 수동 검증을 자동 테스트로 전환
- 수치 오차 허용 범위 설정 (EPSILON)
- **확인**: ctest 통과

### Step 5 — SolutionStep 기록
- SolutionStep 구조체 도입
- det, 역행렬 알고리즘에 step 기록 로직 추가
- main.cpp에서 steps 출력 확인
- **확인**: 풀이 과정이 단계별로 출력됨

### Step 6 (이후) — 레이어 분리
- Step 1~5가 안정되면 Solver, OutputFormatter 등으로 분리
- FTXUI 연동은 calc 기능이 충분히 쌓인 후

## Phase 1 완료 기준

- [ ] Matrix 클래스: 생성, 출력, 사칙연산, 전치, det, 역행렬
- [ ] OutputFormatter: 행렬 박스 렌더링
- [ ] 단위 테스트 통과
- [ ] SolutionStep으로 풀이 과정 기록·출력 가능
