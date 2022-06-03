# deepShell

A simple implementation of a shell in C

## Running

`git clone` to your local linux, use `gcc -o deepShell src/main.c` to compile, and then `./deepShell` to run. 

## Feature 

* builtin funtion support: `cd`, `help`, `history`, `exit`.
```
[deepz@/home/deepz/deepShell] >> help
Welcome to deepz's LSH
Type program name and argument, and hit enter
The following are built in
        cd
        help
        history
        exit
Use the man command for information on other programs
[deepz@/home/deepz/deepShell] >> cd ..
[deepz@/home/deepz] >> pwd
/home/deepz
[deepz@/home/deepz] >> history
0       help
1       cd ..
2       pwd
3       history
[deepz@/home/deepz] >> exit
Bye~
```

* key up/down to view your history command.
* basic command with argument support: `cd`, `ls`, `cat`, `pwd` etc.
* redirect support
```
[root@/home/deepz/deepShell] >> touch test.md
[root@/home/deepz/deepShell] >> echo "hello world" > test.md 
[root@/home/deepz/deepShell] >> cat test.md
"hello world"
```

* piping support
```
[deepz@/home/deepz/deepShell] >> ls -lh
total 56K
-rwxr-xr-x 1 root root  23K Jun  3 16:33 lsh
-rw-r--r-- 1 root root  490 Jun  1 14:18 README.md
drwxr-xr-x 2 root root 4.0K Jun  3 15:29 src
-rwxr-xr-x 1 root root  17K May 31 21:50 test
-rw-r--r-- 1 root root  490 Jun  3 16:26 test.md
[deepz@/home/deepz/deepShell] >> ls -lh | grep test
-rwxr-xr-x 1 root root  17K May 31 21:50 test
-rw-r--r-- 1 root root  490 Jun  3 16:26 test.md
```
## To do feature

* key left/right to move cursor.
* more builtin function support.
