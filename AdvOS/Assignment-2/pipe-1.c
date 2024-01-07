/*
  - Modified pipe read and write to send maximum data possible compared to the original transfer of one byte at a time
  - Sent the minimum of remaining bytes or max space available in the buffer
  - Was able to greatly reduce the ticks from 125ish to 26ish

  (Used chatgpt to understand certain C syntaxes, since I don't have much experience with c and pointers)

*/

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

#define PIPESIZE 512
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

struct pipe {
  struct spinlock lock;
  char data[PIPESIZE];
  uint nread;     // number of bytes read
  uint nwrite;    // number of bytes written
  int readopen;   // read fd is still open
  int writeopen;  // write fd is still open
};

int
pipealloc(struct file **f0, struct file **f1)
{
  struct pipe *pi;

  pi = 0;
  *f0 = *f1 = 0;
  if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
    goto bad;
  if((pi = (struct pipe*)kalloc()) == 0)
    goto bad;
  pi->readopen = 1;
  pi->writeopen = 1;
  pi->nwrite = 0;
  pi->nread = 0;
  initlock(&pi->lock, "pipe");
  (*f0)->type = FD_PIPE;
  (*f0)->readable = 1;
  (*f0)->writable = 0;
  (*f0)->pipe = pi;
  (*f1)->type = FD_PIPE;
  (*f1)->readable = 0;
  (*f1)->writable = 1;
  (*f1)->pipe = pi;
  return 0;

 bad:
  if(pi)
    kfree((char*)pi);
  if(*f0)
    fileclose(*f0);
  if(*f1)
    fileclose(*f1);
  return -1;
}

void
pipeclose(struct pipe *pi, int writable)
{
  acquire(&pi->lock);
  if(writable){
    pi->writeopen = 0;
    wakeup(&pi->nread);
  } else {
    pi->readopen = 0;
    wakeup(&pi->nwrite);
  }
  if(pi->readopen == 0 && pi->writeopen == 0){
    release(&pi->lock);
    kfree((char*)pi);
  } else
    release(&pi->lock);
}

int
pipewrite(struct pipe *pi, uint64 addr, int n)
{
  int i = 0;
  struct proc *pr = myproc();

  acquire(&pi->lock);
  while(i < n){
    if(pi->readopen == 0 || killed(pr)){
      release(&pi->lock);
      return -1;
    }
    if(pi->nwrite == pi->nread + PIPESIZE){ //DOC: pipewrite-full
      wakeup(&pi->nread);
      sleep(&pi->nwrite, &pi->lock);
    } else {
      // Copy either the max bytes that can be written (pipesize - already full buffer) or remaining bytes
      int copy_buff = MIN(PIPESIZE - (pi->nwrite - pi->nread), n - i);
      if (copy_buff <= 0){
        break;
      }
      
      if(copyin(pr->pagetable, pi->data + pi->nwrite % PIPESIZE, addr + i, copy_buff) == -1)
        break;
      pi->nwrite += copy_buff;
      i += copy_buff;
    }
  }
  wakeup(&pi->nread);
  release(&pi->lock);

  return i;
}

int
piperead(struct pipe *pi, uint64 addr, int n)
{
  int i = 0;
  struct proc *pr = myproc();
  // char ch;

  acquire(&pi->lock);
  while(pi->nread == pi->nwrite && pi->writeopen){  //DOC: pipe-empty
    if(killed(pr)){
      release(&pi->lock);
      return -1;
    }
    sleep(&pi->nread, &pi->lock); //DOC: piperead-sleep
  }
  while (i < n && pi->nread != pi->nwrite){
    // Copy either the max bytes that can be read (pipesize - already full buffer) or remaining bytes
    int copy_buff = MIN(PIPESIZE - (pi->nread - pi->nwrite), n - i);
      if (copy_buff <= 0){
        break;
      }

      if(copyout(pr->pagetable, addr + i, pi->data + (pi->nread % PIPESIZE), copy_buff) == -1)
        break;

      pi->nread += copy_buff;
      i += copy_buff;
  }
  

  // for(i = 0; i < n; i++){  //DOC: piperead-copy
  //   if(pi->nread == pi->nwrite)
  //     break;
  //   ch = pi->data[pi->nread++ % PIPESIZE];
  //   if(copyout(pr->pagetable, addr + i, &ch, 1) == -1)
  //     break;
  // }
  wakeup(&pi->nwrite);  //DOC: piperead-wakeup
  release(&pi->lock);
  return i;
}
