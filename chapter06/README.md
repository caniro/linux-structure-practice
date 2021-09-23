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
  $ script/cache_test.sh
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

- 캐시 메모리가 캐시 라인 단위로 데이터를 다루는 것처럼, **페이지 캐시는 페이지 단위로 데이터를 다룬다.**

- 시스템의 메모리가 허용하는 한, 각 프로세스가 페이지 캐시에 없는 파일을 읽을 때마다 페이지 캐시 사이즈가 점점 증가한다.

  - 만약 시스템 메모리가 부족해지면, 커널이 페이지 캐시를 해제한다.

    - 더티 플래그가 없는 페이지부터 해제하며, 그래도 부족하면 더티 페이지를 라이트 백<sub>write back</sub>한 뒤 해제한다. 이 때 저장 장치에 접근하므로 성능 저하가 발생할 수 있다.

### 읽기

프로세스가 파일을 읽으면, 커널 메모리 내의 페이지 캐시 영역에 데이터를 복사한 뒤 이것을 프로세스 메모리에 복사한다.

- 또한 커널 메모리에는 페이지 캐시에 캐싱한 파일과 범위 등의 정보를 저장한다.

- 페이지 캐시는 전체 프로세스의 공유 자원이므로, 여러 프로세스에서 같은 파일을 읽는 경우에도 속도가 향상될 수 있다.

### 쓰기

프로세스가 파일에 쓰기 작업을 하면, 커널은 일단 페이지 캐시에 데이터를 쓴다. 이 때 [캐시 메모리 동작과정](#캐시-메모리-동작-과정)과 같이 더티 플래그를 써넣고, 이 플래그가 있는 페이지를 더티 페이지<sub>dirty page</sub>라고 부른다.

- 더티 페이지 역시 나중에 커널의 백그라운드 처리로 저장 장치에 반영되며, 저장 후 더티 플래그를 지운다.

#### 쓰기 동기화

- 더티 페이지가 존재하는 상태로 시스템이 꺼지면 변경된 데이터가 사라져버린다.

- 따라서 중요한 파일을 열 때는, open() 시스템 콜에서 `O_SYNC` 플래그를 사용하여 수정할 때마다 동기화하여 저장 장치에도 반영하도록 한다. (write through)

### 버퍼 캐시<sub>buffer cache</sub> (7장)

파일시스템을 사용하지 않고 디바이스 파일을 통해 저장 장치에 직접 접근하는 방식이다.

페이지 캐시와 버퍼 캐시를 합쳐서 저장 장치 안의 데이터를 메모리에 넣어둔다.

### 파일 읽기 테스트

1GiB 용량을 가지는 testfile을 생성한다.

```sh
$ dd if=/dev/zero of=testfile oflag=direct bs=1M count=1K
1024+0 레코드 들어옴
1024+0 레코드 나감
1073741824 bytes (1.1 GB, 1.0 GiB) copied, 2.54515 s, 422 MB/s
$ ls -al
합계 1048624
drwxrwxr-x 6 ubun2 ubun2       4096  9월 22 23:51 .
drwxrwxr-x 9 ubun2 ubun2       4096  9월 18 17:18 ..
-rw-rw-r-- 1 ubun2 ubun2      16413  9월 22 23:50 README.md
drwxrwxr-x 2 ubun2 ubun2       4096  9월 18 22:08 log
drwxrwxr-x 2 ubun2 ubun2       4096  9월 18 22:06 output
drwxrwxr-x 2 ubun2 ubun2       4096  9월 22 23:50 script
drwxrwxr-x 2 ubun2 ubun2       4096  9월 18 18:33 src
-rw-rw-r-- 1 ubun2 ubun2 1073741824  9월 22 23:51 testfile
```

  - `oflag=direct`는 쓰기에 페이지 캐시를 사용하지 않는 옵션이다. 이 옵션이 없으면 파일을 생성하면서 페이지 캐시에 영향을 주는 듯

이 시점에서 testfile 파일의 페이지 캐시는 없다. 이 상태에서 testfile을 읽는데 걸리는 시간과 메모리 사용량을 측정하면 다음과 같다.

```sh
$ free 
              total        used        free      shared  buff/cache   available
Mem:        7729028     1916920     3006032      710884     2806076     4818860
스왑:       2097148           0     2097148
$ time cat testfile > /dev/null
cat testfile > /dev/null  0.00s user 0.50s system 44% cpu 1.117 total
$ free
              total        used        free      shared  buff/cache   available
Mem:        7729028     1913976     1956100      713068     3858952     4818608
스왑:       2097148           0     2097148
```

  - 약 1.117초가 걸렸으며, 커널은 0.5초만을 사용했다. 즉 나머지 0.617초는 저장 장치로의 접근 시간이다.

  - 전후의 buff/cache 필드를 살펴보면 페이지 캐시때문에 약 1GiB정도 증가한 것을 볼 수 있다. 

파일을 다시 읽으면 다음과 같다.

```sh
$ time cat testfile > /dev/null
cat testfile > /dev/null  0.00s user 0.13s system 99% cpu 0.134 total
$ free
              total        used        free      shared  buff/cache   available
Mem:        7729028     1946052     1872112      768788     3910864     4730752
스왑:       2097148           0     2097148
```
  - 시간이 훨씬 줄어든 것을 볼 수 있다.

  - 페이지 캐시는 아까에 비해 별 변화가 없다.

또한 페이지 캐시 용량은 다음 명령어의 `kbcached` 필드로 확인할 수 있다. (KiB 단위)

```sh
$ sar -r 1
Linux 5.13.13-surface (ubun2-Surface-Pro-7) 	2021년 09월 22일 	_x86_64_(8 CPU)

23시 57분 55초 kbmemfree   kbavail kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
23시 57분 56초   1909668   4768680   1850712     23.94     80212   3668120   9387676     95.54   1929640   2868344       100
$ rm testfile
```

#### 시스템 통계 정보 확인

- 위에서 했던 작업을 셸 스크립트 파일로 실행하는 동시에, `sar -B` 명령어로 페이지에 대한 정보를 관찰한다.

  ```sh
  $ sar -B 1
  Linux 5.13.13-surface (ubun2-Surface-Pro-7) 	2021년 09월 23일 	_x86_64_(8 CPU)

  19시 58분 34초  pgpgin/s pgpgout/s   fault/s  majflt/s  pgfree/s pgscank/s pgscand/s pgsteal/s    %vmeff
  19시 58분 35초      0.00      0.00   2977.00      0.00  32024.00      0.00      0.00      0.00      0.00
  19시 58분 36초     80.00 214016.00   9429.00      1.00   4048.00      0.00      0.00      0.00      0.00
  19시 58분 37초      0.00 425984.00   6482.00      0.00  15104.00      0.00      0.00      0.00      0.00
  19시 58분 38초      0.00 408708.00   2695.00      0.00   4759.00      0.00      0.00      0.00      0.00
  19시 58분 39초      0.00      0.00     22.00      0.00   4895.00      0.00      0.00      0.00      0.00
  19시 58분 40초      0.00      0.00     39.00      0.00   1038.00      0.00      0.00      0.00      0.00
  19시 58분 41초  18688.00      0.00    324.00      0.00    631.00      0.00      0.00      0.00      0.00
  19시 58분 42초 1029888.00      0.00    419.00      0.00  11895.00      0.00      0.00      0.00      0.00
  19시 58분 43초      0.00     48.00     57.00      0.00   1000.00      0.00      0.00      0.00      0.00
  19시 58분 44초      0.00      0.00      0.00      0.00    322.00      0.00      0.00      0.00      0.00
  19시 58분 45초      0.00      0.00    303.00      0.00    206.00      0.00      0.00      0.00      0.00
  19시 58분 46초      0.00      0.00  12903.00      0.00 268976.00      0.00      0.00      0.00      0.00
  ```

  ```sh
  $ script/read-twice.sh 
  2021. 09. 23. (목) 19:58:36 KST: start file creation
  1024+0 레코드 들어옴
  1024+0 레코드 나감
  1073741824 bytes (1.1 GB, 1.0 GiB) copied, 2.47484 s, 434 MB/s
  2021. 09. 23. (목) 19:58:38 KST: end file creation
  2021. 09. 23. (목) 19:58:38 KST: sleep 3 seconds
  2021. 09. 23. (목) 19:58:41 KST: start 1st read
  2021. 09. 23. (목) 19:58:42 KST: end 1st read
  2021. 09. 23. (목) 19:58:42 KST: sleep 3 seconds
  2021. 09. 23. (목) 19:58:45 KST: start 2nd read
  2021. 09. 23. (목) 19:58:45 KST: end 2nd read
  ```

    - 파일을 생성하는 36초~38초 동안, 페이지 아웃이 1GiB정도 발생한 것을 볼 수 있다. 이는 페이지 캐시를 사용하지 않을 때에도 저장 장치에 파일의 데이터를 쓸 때 페이지 아웃으로 계산하기 때문이다.

    - 41~42초동안 1GiB의 페이지 인이 발생하는 것을 볼 수 있다. testfile의 내용을 페이지 캐시로 불러온 것이다.

    - 45초(두 번째 읽기)에서는 페이지 인이 발생하지 않은 것을 볼 수 있다.

- 이번에는 `sar -d -p` 명령어로 저장 장치에 발생한 I/O의 양을 확인해보자.

  - 일단 mount 명령어로 루트 파일시스템이 존재하는 저장 장치의 이름을 확인한다.

    ```sh
    $ mount | grep "on / "
    /dev/nvme0n1p5 on / type ext4 (rw,relatime,errors=remount-ro)
    ```

  - `sar -d -p` 명령어의 출력 필드는 다음과 같다.

    ```sh
    $ sar -d -p 1
    Linux 5.13.13-surface (ubun2-Surface-Pro-7) 	2021년 09월 23일 	_x86_64_(8 CPU)

    20시 09분 42초       DEV       tps     rkB/s     wkB/s     dkB/s   areq-sz    aqu-sz     await     %util
    20시 09분 43초     loop0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop2      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop3      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop4      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop5      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop6      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop7      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop8      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초     loop9      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초    loop10      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초    loop11      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초       sda      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초    loop12      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 09분 43초    loop13      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    ```

      - rkB/s와 wkB/s가 저장 장치에 대해 읽고 쓴 데이터의 양이다.

      - %util 필드는 측정 시간(1초) 동안 저장 장치에 접근한 시간의 퍼센트량이다.

  - 스크립트 파일을 실행하면서 `sar -d -p` 명령어로 저장 장치의 I/O 발생량을 확인한다.
  
    ```sh
    $ sar -d -p 1 | grep nvme0n1
    20시 10분 37초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 38초   nvme0n1   1094.00      0.00 280064.00      0.00    256.00      1.46      1.34     66.80
    20시 10분 39초   nvme0n1    808.00      0.00 206848.00      0.00    256.00      2.27      2.80    100.00
    20시 10분 40초   nvme0n1    618.00      0.00 158208.00      0.00    256.00      2.34      3.78    100.00
    20시 10분 41초   nvme0n1    680.00      0.00 173592.00      0.00    255.28      2.37      3.46    100.00
    20시 10분 42초   nvme0n1    675.00      0.00 172800.00      0.00    256.00      2.34      3.47    100.00
    20시 10분 43초   nvme0n1    224.00      0.00  57344.00      0.00    256.00      0.80      3.56     34.00
    20시 10분 44초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 45초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 46초   nvme0n1   2692.00 672000.00    976.00      0.00    249.99      1.05      0.39     65.60
    20시 10분 47초   nvme0n1   1471.00 376576.00      0.00      0.00    256.00      0.59      0.40     38.80
    20시 10분 48초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 49초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 50초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 10분 51초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    ```

    ```sh
    $ script/read-twice.sh
    2021. 09. 23. (목) 20:10:37 KST: start file creation
    1024+0 레코드 들어옴
    1024+0 레코드 나감
    1073741824 bytes (1.1 GB, 1.0 GiB) copied, 5.00482 s, 215 MB/s
    2021. 09. 23. (목) 20:10:42 KST: end file creation
    2021. 09. 23. (목) 20:10:42 KST: sleep 3 seconds
    2021. 09. 23. (목) 20:10:45 KST: start 1st read
    2021. 09. 23. (목) 20:10:46 KST: end 1st read
    2021. 09. 23. (목) 20:10:46 KST: sleep 3 seconds
    2021. 09. 23. (목) 20:10:49 KST: start 2nd read
    2021. 09. 23. (목) 20:10:49 KST: end 2nd read
    ```

      - 일단 시간 측정의 오차로 1초씩 밀린 것 같다.

      - 38초~43초동안 파일을 생성하며 1GiB의 쓰기가 발생한 것을 볼 수 있다.

      - 46~47초동안 1GiB의 I/O 읽기가 발생한 것을 볼 수 있다.

      - 49초에는 저장 장치에서 읽기 작업이 일어나지 않은 것을 알 수 있다.

### 파일 쓰기 테스트

위에서와 같이 1GiB의 testfile을 생성할 때, 페이지 캐시 사용 유무에 따른 소요 시간의 차이를 관찰한다.

```sh
$ time dd if=/dev/zero of=testfile oflag=direct bs=1M count=1K 
1024+0 레코드 들어옴
1024+0 레코드 나감
1073741824 bytes (1.1 GB, 1.0 GiB) copied, 2.50148 s, 429 MB/s
dd if=/dev/zero of=testfile oflag=direct bs=1M count=1K  0.00s user 0.26s system 10% cpu 2.504 total
$ rm -f testfile 
$ time dd if=/dev/zero of=testfile bs=1M count=1K
1024+0 레코드 들어옴
1024+0 레코드 나감
1073741824 bytes (1.1 GB, 1.0 GiB) copied, 0.843678 s, 1.3 GB/s
dd if=/dev/zero of=testfile bs=1M count=1K  0.00s user 0.55s system 65% cpu 0.846 total
$ rm -f testfile
```

  - 다이렉트 I/O(페이지 캐시 사용 X)의 경우 2.5초가 걸린다.

  - 페이지 캐시를 사용하였을 경우 약 0.84초가 걸렸다. 2배가 넘게 빠른 셈이다.

#### 시스템 통계 정보 확인

- *write.sh* 스크립트를 실행하며 `sar -B` 명령어로 시스템의 통계 정보를 확인한다.

- 책의 내용과 달리 내 환경에서는 페이지 아웃이 발생한다. 그래서 다이렉트 I/O와 페이지 아웃 발생량을 비교해본다.

  - 페이지 캐시 사용 X

    ```sh
    $ sar -B 1
    Linux 5.13.13-surface (ubun2-Surface-Pro-7) 	2021년 09월 23일 	_x86_64_	(8 CPU)

    20시 36분 30초  pgpgin/s pgpgout/s   fault/s  majflt/s  pgfree/s pgscank/s pgscand/s pgsteal/s    %vmeff
    20시 36분 31초      0.00      0.00     97.00      0.00    698.00      0.00      0.00      0.00      0.00
    20시 36분 32초      0.00 181248.00   4202.00      0.00  17727.00      0.00      0.00      0.00      0.00
    20시 36분 33초      0.00 423936.00   3791.00      0.00   9221.00      0.00      0.00      0.00      0.00
    20시 36분 34초      0.00 419840.00   1660.00      0.00   1759.00      0.00      0.00      0.00      0.00
    20시 36분 35초      0.00  23552.00   6978.00      0.00   6528.00      0.00      0.00      0.00      0.00
    20시 36분 36초      0.00      0.00   2855.00      0.00   6085.00      0.00      0.00      0.00      0.00
    ```

    ```sh
    $ script/write-direct.sh
    2021. 09. 23. (목) 20:36:31 KST: start write (file creation)
    1024+0 레코드 들어옴
    1024+0 레코드 나감
    1073741824 bytes (1.1 GB, 1.0 GiB) copied, 2.74401 s, 391 MB/s
    2021. 09. 23. (목) 20:36:34 KST: end write
    ```

  - 페이지 캐시 사용 O

    ```sh
    $ sar -B 1
    Linux 5.13.13-surface (ubun2-Surface-Pro-7) 	2021년 09월 23일 	_x86_64_	(8 CPU)

    20시 37분 34초  pgpgin/s pgpgout/s   fault/s  majflt/s  pgfree/s pgscank/s pgscand/s pgsteal/s    %vmeff
    20시 37분 35초      0.00      0.00    736.00      0.00   3488.00      0.00      0.00      0.00      0.00
    20시 37분 36초      0.00 236760.00   5643.00      0.00  15954.00      0.00      0.00      0.00      0.00
    20시 37분 37초      0.00 189244.00   3852.00      0.00 266577.00      0.00      0.00      0.00      0.00
    20시 37분 38초      0.00      0.00   8121.00      0.00  23635.00      0.00      0.00      0.00      0.00
    20시 37분 39초      0.00      0.00      1.00      0.00   1057.00      0.00      0.00      0.00      0.00
    ```

    ```sh
    $ script/write.sh
    2021. 09. 23. (목) 20:37:36 KST: start write (file creation)
    1024+0 레코드 들어옴
    1024+0 레코드 나감
    1073741824 bytes (1.1 GB, 1.0 GiB) copied, 0.768723 s, 1.4 GB/s
    2021. 09. 23. (목) 20:37:36 KST: end write
    ```

      - 페이지 아웃이 발생하긴 했으나, 다이렉트 I/O의 경우보다 절반도 안되는 양임을 확인할 수 있다.

- `sar -d -p` 명령어로 저장 장치의 I/O 발생량을 확인한다. (4번째 필드가 wkB/s)

  - 페이지 캐시 사용 X

    ```sh
    $ sar -d -p 1 | grep nvme0n1
    20시 39분 21초   nvme0n1     11.00      0.00     52.00      0.00      4.73      0.04      2.00      2.40
    20시 39분 22초   nvme0n1   1156.00      0.00 295936.00      0.00    256.00      1.54      1.33     70.40
    20시 39분 23초   nvme0n1    960.00      0.00 245760.00      0.00    256.00      2.29      2.38    100.00
    20시 39분 24초   nvme0n1    596.00      0.00 152576.00      0.00    256.00      2.34      3.93    100.00
    20시 39분 25초   nvme0n1    662.00      0.00 169296.00      0.00    255.73      2.32      3.51    100.00
    20시 39분 26초   nvme0n1    725.00      0.00 167868.00      0.00    231.54      2.59      3.55    100.00
    20시 39분 27초   nvme0n1     69.00      0.00  17664.00      0.00    256.00      0.25      3.59     10.40
    20시 39분 28초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    ```

    ```sh
    $ script/write-direct.sh    
    2021. 09. 23. (목) 20:39:21 KST: start write (file creation)
    1024+0 레코드 들어옴
    1024+0 레코드 나감
    1073741824 bytes (1.1 GB, 1.0 GiB) copied, 4.80191 s, 224 MB/s
    2021. 09. 23. (목) 20:39:26 KST: end write
    ```

  - 페이지 캐시 사용

    ```sh
    $ sar -d -p 1 | grep nvme0n1
    20시 38분 47초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    20시 38분 48초   nvme0n1    206.00      0.00  45236.00      0.00    219.59      2.64     12.82     23.20
    20시 38분 49초   nvme0n1    744.00      0.00 180920.00      0.00    243.17    205.91    276.76    100.00
    20시 38분 50초   nvme0n1    379.00      0.00  93708.00      0.00    247.25    348.13    918.41     73.60
    20시 38분 51초   nvme0n1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
    ```

    ```sh
    $ script/write.sh
    2021. 09. 23. (목) 20:38:47 KST: start write (file creation)
    1024+0 레코드 들어옴
    1024+0 레코드 나감
    1073741824 bytes (1.1 GB, 1.0 GiB) copied, 1.17943 s, 910 MB/s
    2021. 09. 23. (목) 20:38:49 KST: end write
    ```

      - 저장 장치에 I/O가 발생하긴 했으나, 다이렉트 I/O의 경우보다 발생량이 적은 것을 알 수 있다.

### 튜닝 파라미터

리눅스에는 페이지 캐시를 제어하기 위한 튜닝 파라미터들이 존재한다.

#### sysctl의 `vm.dirty_writeback_centisecs`

- 더티 페이지의 라이트 백이 발생하는 주기 (단위 : 1/100초)

- 값을 0으로 하면 주기적인 라이트 백은 발생하지 않지만 위험하다고 한다.

- 기본값 : 5초에 1번

  ```sh
  $ sysctl vm.dirty_writeback_centisecs
  vm.dirty_writeback_centisecs = 500
  ```

#### sysctl의 `vm.dirty_background_ratio`

- 시스템의 물리 메모리 중 더티 페이지가 차지하는 최대 비율

- 더티 페이지가 이 값(퍼센트)을 초과할 경우 백그라운드 라이트 백 처리가 동작한다.

- 메모리가 부족할 때 라이트 백 부하가 커지는 것을 방지한다.

- 기본값 : 10 (10%)

  ```sh
  $ sysctl vm.dirty_background_ratio
  vm.dirty_background_ratio = 10
  ```

#### sysctl의 `vm.dirty_background_bytes`

- vm.dirty_background_ratio를 비율 대신 바이트 단위로 설정할 때 사용한다.

- 기본값 : 0 (사용하지 않음)

  ```sh
  $ sysctl vm.dirty_background_bytes
  vm.dirty_background_bytes = 0
  ```

#### sysctl의 `vm.dirty_ratio`

- 더티 페이지가 차지하는 비율이 이 값(퍼센트)을 초과하면, 프로세스에 의한 파일에 쓰기의 연장으로 동기적인 라이트 백을 수행한다.

- 기본값 : 20

  ```sh
  $ sysctl vm.dirty_ratio
  vm.dirty_ratio = 20
  ```

#### sysctl의 `vm.dirty_bytes`

- vm.dirty_ratio를 비율 대신 바이트 단위로 설정할 때 사용한다.

- 기본값 : 0 (사용하지 않음)

  ```sh
  $ sysctl vm.dirty_bytes      
  vm.dirty_bytes = 0
  ```

#### 시스템의 페이지 캐시 비우기

- `/proc/sys/vm/drop_caches` 라는 파일에 3을 넣으면 페이지 캐시가 비워진다고 한다.

  ```sh
  $ sudo su
  ubun2-Surface-Pro-7# free   
                total        used        free      shared  buff/cache   available
  Mem:        7729020     1409952     3616468      526436     2702600     5506612
  스왑:       2097148           0     2097148
  ubun2-Surface-Pro-7# echo 3 > /proc/sys/vm/drop_caches 
  ubun2-Surface-Pro-7# free
                total        used        free      shared  buff/cache   available
  Mem:        7729020     1404616     5333292      527360      991112     5546864
  스왑:       2097148           0     2097148
  ```

    - 왜인지 zsh에서는 sudo 명령어로도 덮어쓰기가 안돼서 루트 유저로 진행했다.

    - 페이지 캐시가 약 1.7GiB 비워진 것을 볼 수 있다.

### 정리

- 파일의 데이터가 페이지 캐시에 있으면 파일 접근이 굉장히 빨라진다.

- 설정을 변경했거나 시간이 지나면서 시스템 성능이 안좋아졌을 경우, 파일의 데이터가 페이지 캐시에 제대로 들어가는지 확인하는게 좋다.

- sysctl 파라미터들을 잘 튜닝하면 페이지 캐시의 라이트 백 I/O 부하를 예방할 수 있다.

- `sar -B`, `sar -d -p` 명령어를 통해 페이지 캐시에 관한 통계를 얻을 수 있다.

---

## 하이퍼스레드<sub>hyper-thread</sub>

CPU의 계산은 굉장히 빠른 것에 비해, 메모리 및 캐시 메모리의 접근 레이턴시는 상대적으로 느리다. 따라서 CPU 사용 시간 중 대부분은 메모리나 캐시 메모리로부터 데이터를 기다리는 시간이라고 할 수 있다.

하이퍼스레드 기능은 CPU 코어 1개의 레지스터 등 일부 자원을 2개씩(일반적) 구성하여 2개의 논리 CPU(하이퍼스레드라는 단위)로 인식되도록 하는 하드웨어의 기능이다.

4코어 8스레드의 CPU라고 해도 4코어 4스레드인 CPU에 비해 2배 성능이 나오는 것은 아니고, 현실적으로 20~30%의 성능 향상이 나오면 훌륭한 것이라고 한다.

책의 저자가 커널 빌드에 걸리는 시간을 기준으로 테스트했을 때, 하이퍼 스레드를 끈 경우에 비해 켰을 경우 22%(93초 -> 73초)의 성능 향상이 있었다. 즉 생각처럼 2배만큼 빨라지는 것은 아니다.

하이퍼스레드의 짝을 이루는 논리 CPU는 */sys/devices/system/cpu/cpu0/topology/thread_siblings_list* 파일에서 볼 수 있다. cpu0 대신 보고 싶은 논리 CPU의 번호를 넣으면 된다.

```sh
$ ls /sys/devices/system/cpu/cpu*/topology/thread_siblings_list
/sys/devices/system/cpu/cpu0/topology/thread_siblings_list
/sys/devices/system/cpu/cpu1/topology/thread_siblings_list
/sys/devices/system/cpu/cpu2/topology/thread_siblings_list
/sys/devices/system/cpu/cpu3/topology/thread_siblings_list
/sys/devices/system/cpu/cpu4/topology/thread_siblings_list
/sys/devices/system/cpu/cpu5/topology/thread_siblings_list
/sys/devices/system/cpu/cpu6/topology/thread_siblings_list
/sys/devices/system/cpu/cpu7/topology/thread_siblings_list
$ cat < $(ls /sys/devices/system/cpu/cpu*/topology/thread_siblings_list)
0,4
1,5
2,6
3,7
0,4
1,5
2,6
3,7
```

하이퍼스레드를 켰을 때 오히려 성능 저하가 발생하는 경우도 있다고 하는데, 아마 계산 시간이 길면 오버헤드가 더 커져서 스래싱같은 상황이 발생할 수도 있겠다고 생각한다.
