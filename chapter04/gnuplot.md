# Gnuplot

- 그래프를 그리는 툴로 유명한 gnuplot을 살짝 사용해보았다.

---

## 설치

- 우분투 20.04 기준, 패키지 매니저로 설치할 수 있다.

  ```sh
  $ sudo apt install -y gnuplot
  ```

---

## 사용법

### 기본 조작

- 다음과 같이 함수를 통해 출력할 수 있다.

  ```gnuplot
  $ gnuplot

          G N U P L O T
          Version 5.2 patchlevel 8    last modified 2019-12-01 

          Copyright (C) 1986-1993, 1998, 2004, 2007-2019
          Thomas Williams, Colin Kelley and many others

          gnuplot home:     http://www.gnuplot.info
          faq, bugs, etc:   type "help FAQ"
          immediate help:   type "help"  (plot window: hit 'h')

  Terminal type is now 'qt'
  gnuplot> plot sin(x)
  gnuplot> unset key                          # 그래프의 레이블을 지운다. (sin(x))
  gnuplot> replot
  gnuplot> plot [x=-5:5] sin(x)               # x축을 -5부터 5까지만 표시한다.
  gnuplot> plot [-10:10] cos(x)               # "x=" 표시를 생략할 수 있다.
  gnuplot> plot [-10:10] [-0.5:0.5] cos(x)    # y축도 설정할 수 있다.
  gnuplot> set grid                           # 격자 표시
  gnuplot> replot
  gnuplot> set xlabel "Elapsed time[ms]"      # x축 레이블 설정
  gnuplot> set ylabel "Progress[%]"           # y축 레이블 설정
  gnuplot> set title "1core-1process"         # 그래프 제목 설정
  gnuplot> replot
  ```

- 마우스 조작

  - 휠클릭으로 점을 찍어둘 수 있다.

  - 우클릭으로 영역을 지정하여 확대할 수 있다.

  - 위 패널의 autoscale(돋보기 3번째)를 클릭하여 화면에 꽉차도록 볼 수 있다. 

### 파일의 데이터 출력

- 파일을 기반으로 사용할 열을 지정하여 출력할 수 있다.

  ```gnuplot
  gnuplot> plot "log/1core-1process.log" using 2:3  # 파일의 2열을 x축, 3열을 y축으로 사용
  ```

### 스크립트 실행

- 셸에서 `gnuplot 스크립트파일명` 형태로 바로 실행할 수 있다.

  ```sh
  $ gnuplot plot/1core-1process.gnu
  ```

### 간단한 조건문 예시

- 열의 값으로 조건을 걸어서 원하는 데이터만 출력할 수 있다.

  ```gnuplot
  gnuplot> filename="log/1core-4process.log"
  gnuplot> plot filename using 2:($1=="0"?$3:1/0) title "Process 0"
  ```

    - filename 변수를 설정하고, 그래프로 나타낸다.

    - $1은 첫 번째 열의 값을 나타내고, 삼항 연산자로 조건문을 만들 수 있다.
    
      - 값이 "0"이면 3번째 열을 표시한다.

      - "0"이 아니면 표시하지 않는다. (1/0 : 표시 X)

    - title : 해당 그래프의 key로 나타낼 명칭을 설정한다.

---

### 참고

- <http://coffeenix.net/doc/gnuplot/gnuplot.html>

- <https://m31phy.tistory.com/169>

- <http://gnuplot.info/docs_4.2/node99.html>

- <https://blog.daum.net/wh1988ha/120>
