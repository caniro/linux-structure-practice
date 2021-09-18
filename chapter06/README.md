# Chapter 6. 메모리 계층

## 캐시 메모리

컴퓨터의 기본적인 동작 흐름

1. 메모리에서 레지스터로 데이터를 읽는다.

2. 레지스터의 데이터로 계산한다.

3. 계산 결과를 메모리에 쓴다.

요즘의 하드웨어는 레지스터에서 계산하는 시간(위에서 2)보다 메모리에 접근하는 데 걸리는 시간(1, 3)이 훨씬 느리다. 따라서 1, 3의 느린 속도때문에 병목 현상이 생겨서 전체적인 레이턴시가 증가한다(느려진다).

캐시 메모리는 레지스터와 메모리 사이에서 중간 역할을 하여 1, 3의 속도를 고속화한다. 일반적으로 CPU에 내장되어 있지만, CPU 바깥에 있는 캐시 메모리도 존재한다. 또한 일부 경우(특정 타이밍에 커널이 캐시를 파기하는 등)를 제외하면, 캐시 메모리의 처리는 하드웨어에서 처리된다.

### 캐시 메모리 동작 과정

- 메모리에서 먼저 캐시 메모리로 캐시 라인 사이즈<sub>cache line size</sub>만큼(CPU에서 정함) 읽어오고, 이를 다시 레지스터로 읽어온다. 다음에 같은 메모리 주소에서 데이터를 읽을 경우, 캐시 메모리에서 읽으면 되므로 속도가 향상된다.

- 만약 값을 변경하여 메모리에 입력해야 할 경우, 일단 레지스터로부터 변경된 데이터를 캐시 메모리에 옮긴다(이 때도 캐시 라인 사이즈 단위로). 이 떄 캐시 라인에, 메모리로부터 읽은 데이터가 변경되었음을 나타내는 더티<sub>dirty</sub> 플래그를 추가한다. 더티 플래그가 있으면 나중에 백그라운드 처리로 메모리에 기록하고 더티 플래그를 제거한다. 

### 캐시 메모리가 가득 찬 경우

캐시 메모리가 꽉 찬 상태에서 캐시 메모리에 없는 메모리 주소를 읽으면, 기존의 캐시 메모리 중 1개를 파기한다. 만약 파기하는 데이터가 더티 상태이면, 해당 캐시 라인을 메모리에 동기화시킨 뒤 버린다. 더 나아가 캐시 메모리가 가득 차고 모두 더티 상태라면, 다른 메모리에 접근할 때마다 캐시 라인의 데이터가 자주 바뀌게 되고 이를 **스래싱**이라고 한다. 이러면 오히려 성능이 크게 감소하게 된다.

- 즉 스래싱은 원래의 의도와 달리 오버헤드가 더 커서 성능이 오히려 낮아지는 현상이라고 할 수 있다.

### 계층형 캐시 메모리

최근 x86_64 아키텍처의 CPU는 캐시 메모리가 계층형 구조(L1, L2, L3, L은 Level)로 되어 있으며, 각 계층은 사이즈, 레이턴시, 논리 CPU 사이의 공유 관계 등 차이가 있다. 레지스터와 가장 가까운 L1 캐시가 제일 빠르며 용량은 적다.

#### 캐시 메모리의 정보

- `/sys/devices/system/cpu/cpu0/cache/index0/` 디렉토리의 파일들이 캐시 메모리에 대한 정보를 가지고 있다.

  - cpu0과 index0의 숫자를 바꿔가며 각 논리 CPU 및 계층형 캐시에 대한 정보를 볼 수 있다.

- 먼저 전체 캐시 메모리에 대한 정보를 얻어보자. 내 CPU에 대한 정보는 상위 디렉토리의 *README.md* 에 작성되어 있다.

  ```sh
  $ lscpu | grep cache                              
  L1d cache:                       192 KiB
  L1i cache:                       128 KiB
  L2 cache:                        2 MiB
  L3 cache:                        6 MiB
  ```

    - L1d는 데이터를 위한 L1 캐시 메모리이고, L1i는 명령어(intstruction)를 위한 L1 캐시 메모리 공간이다.

    - 위에서 설명한 대로 L1쪽의 크기가 작은 것을 알 수 있다.

- 이제 논리 CPU 0번의 index0 캐시 메모리에 대한 정보를 파악해보자.

  ```sh
  $ head /sys/devices/system/cpu/cpu0/cache/index0/*
  ==> /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size <==
  64 # 캐시 라인 사이즈

  ==> /sys/devices/system/cpu/cpu0/cache/index0/id <==
  0 # 캐시메모리 자체의 id. 내 경우 4코어 8스레드이고 2스레드씩 캐시를 공유하여 0~3의 값을 갖는 것을 확인했다.

  ==> /sys/devices/system/cpu/cpu0/cache/index0/level <==
  1 # Level 1 캐시임을 의미

  ==> /sys/devices/system/cpu/cpu0/cache/index0/number_of_sets <==
  64

  ==> /sys/devices/system/cpu/cpu0/cache/index0/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu0/cache/index0/shared_cpu_list <==
  0,4 # 이 캐시 메모리를 공유하는 논리 CPU의 번호

  ==> /sys/devices/system/cpu/cpu0/cache/index0/shared_cpu_map <==
  11

  ==> /sys/devices/system/cpu/cpu0/cache/index0/size <==
  48K # 이 캐시 메모리의 사이즈. 총 4캐의 캐시 메모리이므로 4를 곱하면 총 L1d 캐시의 사이즈는 192KiB임을 알 수 있다.

  ==> /sys/devices/system/cpu/cpu0/cache/index0/type <==
  Data # 데이터용 캐시 메모리. 즉 이 캐시 메모리는 L1d 캐시이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index0/uevent <==

  ==> /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity <==
  12
  ```

- 다음은 cpu0의 index1이다.

  ```sh
  $ head /sys/devices/system/cpu/cpu0/cache/index1/*
  ==> /sys/devices/system/cpu/cpu0/cache/index1/coherency_line_size <==
  64

  ==> /sys/devices/system/cpu/cpu0/cache/index1/id <==
  0

  ==> /sys/devices/system/cpu/cpu0/cache/index1/level <==
  1 # 위의 캐시와 같은 L1 캐시이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index1/number_of_sets <==
  64

  ==> /sys/devices/system/cpu/cpu0/cache/index1/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu0/cache/index1/shared_cpu_list <==
  0,4

  ==> /sys/devices/system/cpu/cpu0/cache/index1/shared_cpu_map <==
  11

  ==> /sys/devices/system/cpu/cpu0/cache/index1/size <==
  32K # 위와 다르게 캐시 당 32KiB이며, 4를 곱하면 총 L1i 캐시의 사이즈는 128KiB이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index1/type <==
  Instruction # 이번에는 명령어용 캐시, 즉 L1i 캐시임을 알 수 있다.

  ==> /sys/devices/system/cpu/cpu0/cache/index1/uevent <==

  ==> /sys/devices/system/cpu/cpu0/cache/index1/ways_of_associativity <==
  8
  ```

- cpu0의 index2

  ```sh
  $ head /sys/devices/system/cpu/cpu0/cache/index2/*
  ==> /sys/devices/system/cpu/cpu0/cache/index2/coherency_line_size <==
  64

  ==> /sys/devices/system/cpu/cpu0/cache/index2/id <==
  0

  ==> /sys/devices/system/cpu/cpu0/cache/index2/level <==
  2 # 이번에는 L2 캐시이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index2/number_of_sets <==
  1024

  ==> /sys/devices/system/cpu/cpu0/cache/index2/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu0/cache/index2/shared_cpu_list <==
  0,4

  ==> /sys/devices/system/cpu/cpu0/cache/index2/shared_cpu_map <==
  11

  ==> /sys/devices/system/cpu/cpu0/cache/index2/size <==
  512K # 사이즈가 꽤 커진 것을 볼 수 있다. 4를 곱하면 총 L2 캐시의 사이즈는 2MiB이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index2/type <==
  Unified # 이번에는 데이터나 명령어가 아닌 Unified, 즉 데이터와 명령어 전부를 저장할 수 있는 영역임을 의미한다.

  ==> /sys/devices/system/cpu/cpu0/cache/index2/uevent <==

  ==> /sys/devices/system/cpu/cpu0/cache/index2/ways_of_associativity <==
  8
  ```

- cpu0의 마지막(1035G4는 L3캐시까지 있으므로) index3 캐시이다.

  ```sh
  $ head /sys/devices/system/cpu/cpu0/cache/index3/*
  ==> /sys/devices/system/cpu/cpu0/cache/index3/coherency_line_size <==
  64

  ==> /sys/devices/system/cpu/cpu0/cache/index3/id <==
  0

  ==> /sys/devices/system/cpu/cpu0/cache/index3/level <==
  3 # L3 캐시이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index3/number_of_sets <==
  8192

  ==> /sys/devices/system/cpu/cpu0/cache/index3/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu0/cache/index3/shared_cpu_list <==
  0-7 # 이번에는 논리 CPU 0부터 7까지 전부 공유한다. 즉 L3 캐시는 1개밖에 없다.

  ==> /sys/devices/system/cpu/cpu0/cache/index3/shared_cpu_map <==
  ff

  ==> /sys/devices/system/cpu/cpu0/cache/index3/size <==
  6144K # L3 캐시는 약 6MiB이다.

  ==> /sys/devices/system/cpu/cpu0/cache/index3/type <==
  Unified # 역시 데이터와 명령어 모두 저장할 수 있다.

  ==> /sys/devices/system/cpu/cpu0/cache/index3/uevent <==

  ==> /sys/devices/system/cpu/cpu0/cache/index3/ways_of_associativity <==
  12
  ```

- 이번에는 cpu1의 index0을 보자.

  ```sh
  $ head /sys/devices/system/cpu/cpu1/cache/index0/*
  ==> /sys/devices/system/cpu/cpu1/cache/index0/coherency_line_size <==
  64

  ==> /sys/devices/system/cpu/cpu1/cache/index0/id <==
  1 # 이번에는 캐시 메모리의 id가 1이다.

  ==> /sys/devices/system/cpu/cpu1/cache/index0/level <==
  1

  ==> /sys/devices/system/cpu/cpu1/cache/index0/number_of_sets <==
  64

  ==> /sys/devices/system/cpu/cpu1/cache/index0/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu1/cache/index0/shared_cpu_list <==
  1,5 # 공유하는 논리 CPU가 1번과 5번임을 알 수 있다.

  ==> /sys/devices/system/cpu/cpu1/cache/index0/shared_cpu_map <==
  22

  ==> /sys/devices/system/cpu/cpu1/cache/index0/size <==
  48K

  ==> /sys/devices/system/cpu/cpu1/cache/index0/type <==
  Data

  ==> /sys/devices/system/cpu/cpu1/cache/index0/uevent <==

  ==> /sys/devices/system/cpu/cpu1/cache/index0/ways_of_associativity <==
  12
  ```

- cpu7의 index0은 다음과 같다.

  ```sh
  $ head /sys/devices/system/cpu/cpu7/cache/index0/*
  ==> /sys/devices/system/cpu/cpu7/cache/index0/coherency_line_size <==
  64

  ==> /sys/devices/system/cpu/cpu7/cache/index0/id <==
  3 # L1d 캐시 메모리는 4개이므로 3번인 것을 확인할 수 있다.

  ==> /sys/devices/system/cpu/cpu7/cache/index0/level <==
  1

  ==> /sys/devices/system/cpu/cpu7/cache/index0/number_of_sets <==
  64

  ==> /sys/devices/system/cpu/cpu7/cache/index0/physical_line_partition <==
  1

  ==> /sys/devices/system/cpu/cpu7/cache/index0/shared_cpu_list <==
  3,7 # 논리 CPU 3번과 7번이 공유한다.

  ==> /sys/devices/system/cpu/cpu7/cache/index0/shared_cpu_map <==
  88

  ==> /sys/devices/system/cpu/cpu7/cache/index0/size <==
  48K

  ==> /sys/devices/system/cpu/cpu7/cache/index0/type <==
  Data

  ==> /sys/devices/system/cpu/cpu7/cache/index0/uevent <==

  ==> /sys/devices/system/cpu/cpu7/cache/index0/ways_of_associativity <==
  12
  ```

- 4장에서 논리 CPU 0번과 4번으로 테스트했었는데, 이 책 저자의 경우 L3 캐시가 논리 CPU 0-3, 4-7 이 공유 상태였고, L1과 L2 캐시는 논리 CPU 별로 각각 존재했다. 내 경우에는 L1, L2캐시가 각각 논리 CPU 0,4 1,5 2,6 3,7 에서 공유하므로.. 4장의 테스트를 나중에 다시 해봐야 할 듯 하다ㅠ

### 캐시 실험

- 최적화를 해야 결과를 더 눈에 띄게 알 수 있다고 하니, 컴파일 옵션에 O3 최적화를 추가한다.

  ```sh
  $ gcc -O3 -o output/cache src/cache.c
  $ ./cache_test.sh
  ```

  *log/cache.log*
  ```log
  4[KB] : 0.420382
  8[KB] : 0.367063
  16[KB] : 0.343080
  32[KB] : 0.325464
  64[KB] : 0.886360     # 여기
  128[KB] : 0.889040
  256[KB] : 0.892584
  512[KB] : 1.106641
  1024[KB] : 1.495455   # 여기
  2048[KB] : 1.510899
  4096[KB] : 1.661219
  8192[KB] : 2.780471   # 여기
  16384[KB] : 3.925238
  32768[KB] : 4.266433
  ```

    - 정밀하게 보기는 어렵지만, 접근 시간이 확 뛰는 구간이 캐시 메모리들의 용량과 관련이 있음을 대략적으로 알 수 있다.

    - 32KB 이하에서 성능이 더 좋은 것은 프로그램의 정밀도 문제이므로 신경쓰지 않아도 된다. 어셈블리어로 코드를 작성하면 해결할 수 있다.

### 메모리 참조의 국소성<sub>locality of reference</sub>

- 시간 국소성 : 한 번 접근한 데이터는 가까운 미래에 다시 접근할 가능성이 크다. 

  ex. 반복문의 코드 등

- 공간 국소성 : 어떤 데이터에 접근하면 그 데이터와 가까운 주소에 있는 데이터에 접근할 가능성이 크다.

  ex. 배열 등

위와 같은 이유로, 캐시는 프로세스가 요구한 것보다 더 많은 데이터를 메모리에서 가져와놓고 cache hit를 높인다.

### 정리

프로그램의 성능을 향상시키는 방법 중 하나는, 워크로드를 캐시 메모리 사이즈에 들어가게 하는 것이다. 데이터 배열이나 알고리즘 등을 통해 단위 시간 당 메모리 접근 범위를 작게 하면 도움이 된다.

- 예를 들어 [행렬을 이용한 레이캐스팅 엔진](https://lodev.org/cgtutor/raycasting.html#Performance)에서 x, y 좌표를 나타내는 2차원 배열에 이중반복문을 돌릴 때, 어떤 순서로 반복하느냐가 locality에 영향을 미친다.

또한 시스템 설정을 변경했을 때 같은 프로그램의 성능이 크게 나빠진 경우, 프로그램의 데이터가 캐시 메모리에 전부 들어가지 않았을 가능성이 존재한다.

---

## Translation Lookaside Buffer<sub>TLB</sub>

CPU에 존재하는 가상 주소 변환 고속화 장치이다.

TLB가 없다면, 프로세스가 가상 주소의 데이터에 접근하는 순서는 다음과 같다.

1. 물리 메모리상에 존재하는 페이지 테이블을 참고하여 가상 주소를 물리 주소로 변환한다.

2. 1에서 구한 물리 메모리에 접근한다.

이 때 1에서 물리 메모리상에 있는 페이지 테이블에 접근해야 하고, 이는 캐시 메모리로 고속화할 수 없다. 따라서 가상 주소에서 물리 주소로의 변환표를 CPU에 저장하는데, 캐시 메모리처럼 고속으로 접근 가능하도록 만든 것이 TLB이다.

---

## 페이지 캐시

CPU에서 저장 장치에 접근하려면 굉장히 느리기 때문에 커널은 페이지 캐시 기능을 사용한다. 페이지 캐시는 캐시 메모리의 동작과 매우 비슷하다.

- 캐시 메모리가 메모리의 데이터를 캐시 메모리에 캐싱하는 것처럼, 페이지 캐시는 저장 장치 내의 파일 데이터를 메모리에 캐싱한다.

- 캐시 메모리가 캐시 라인 단위로 데이터를 다루는 것처럼, 페이지 캐시는 페이지 단위로 데이터를 다룬다.
