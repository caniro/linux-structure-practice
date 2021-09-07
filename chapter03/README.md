# 3장 프로세스 관리 실습

## fork() 함수

### fork.c 프로그램 예제

1. 프로세스를 새로 만든다.
2. 부모 프로세스는 자신의 PID<sub>Process ID</sub>와 자식 프로세스의 PID를 출력한다.

```sh
$ gcc -o output/fork src/fork.c
$ output/fork
부모 프로세스의 PID : 13202, 자식 프로세스의 PID : 13203
자식 프로세스의 PID : 13203
```

- 부모 프로세스(13202)가 분기되어 자식 프로세스(13203)이 생성된 것을 볼 수 있고, 각각의 출력을 통해 두 프로세스 처리가 분기되어 실행되는 것을 알 수 있다.

---

## execve() 함수

### fork and exec

- 전혀 다른 프로세스를 생성할 때는, 부모가 될 프로세스로부터 fork() 함수를 호출한 다음 자식 프로세스가 exec() 함수를 호출하는 방식을 주로 사용한다.

  ```sh
  $ gcc -o output/fork-and-exec src/fork-and-exec.c
  $ output/fork-and-exec
  부모 프로세스의 PID : 18241, 자식 프로세스의 PID : 18242
  자식 프로세스의 PID : 18242
  hello
  ```

- 파이썬에서는 **OS.exec()** 함수를 통해 execve() 함수를 호출할 수 있다.
