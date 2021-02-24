#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int
ls(char *path, char *target,int decal)
{
    char realpath[100];
    char buf[100], *p;
    int fd, skip = 2;
    struct dirent de;
    struct stat st;
    int success = 0;
    // printf("%s\n",path);
    if((fd = open(path, 0)) < 0){
        printf("tree: cannot open %s\n", path);
        return success;
    }

    if(fstat(fd, &st) < 0){
        printf("tree: cannot stat %s\n", path);
        close(fd);
        return success;
    }

    switch(st.type){
        // case T_FILE:
        //     printf("%s\n","file");
        //     if(strcmp( fmtname(path), target) == 0){
        //         printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
        //     }
        //     break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("tree: path too long\n");
                break;
            }
            strcpy(realpath, path);
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    printf("tree: cannot stat %s\n", buf);
                    continue;
                }
               
                if (skip){
                    skip--;
                }
                else{
                    // printf("%s %s\n",fmtname(buf),target);
                    if(strcmp( fmtname(buf), target)==0){
                        success = 1;
                        // <pid> as Watson: ./hello 
                        printf("%d as Watson: %s/%s\n", getpid(),realpath,fmtname(buf));
                    }
                    ls(buf, target, decal+1);
                    
                }
            }
            break;
    }
    close(fd);
    return success;
}
int
main(int argc, char *argv[])
{
  int fd[2];
  char readbuffer[10];
  pipe(fd);
  int *write_fd = &fd[1];
  int *read_fd = &fd[0];
  if(argc < 2){
    printf("%s","Usage:detectvie What you want to find.\n");
    exit(0);
  }
  int pid = fork();
  if(pid > 0){
    read(*read_fd,readbuffer,sizeof(readbuffer)-1);
    if(strcmp(readbuffer,"n") == 0){
        // printf("%s",readbuffer);
        printf("%d as Holmes: This is the abili\n",getpid());
    }else if(strcmp(readbuffer,"y") == 0){
        // printf("%s",readbuffer);
        printf("%d as Holmes: This is the evidence\n",getpid());
    }
    pid = wait((int *) 0);
  }else if(pid == 0){
    // printf("child: exiting\n");
    if(ls(".",argv[1],1)){
        write(*write_fd,"y",strlen("y"));
    }else{
        write(*write_fd,"n",strlen("n"));
    }
    exit(0);
  }else {
    printf("fork error\n");
  }
//   printf("child %d is done\n", pid);
  
  exit(0);
}
