#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


// this function is just like malloc, but the memory
// is not copied by forking the program
static void *alloc_shareable_memory(size_t n)
{
	// note MAP_ANONYMOUS is nicer, but less portable than /dev/zero
	//
	//void *r = mmap(NULL, n, PROT_READ|PROT_WRITE,
	//MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	//
	int fd = open("/dev/zero", O_RDWR);
	if (fd == -1) { perror("open(\"/dev/zero\")"); exit(42); }
	void *r = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (r == MAP_FAILED) { perror("mmap"); exit(43); }
	if (close(fd)) { perror("close(\"/dev/zero\")"); exit(44); }
	return r;
}

static void free_shareable_memory(void *p, size_t n)
{
	int r = munmap(p, n);
	if (r == -1) { perror("munmap"), exit(45); }
}

int main()
{
	int n = 300*1024*1024;
	float *tab = alloc_shareable_memory(n*sizeof*tab);

	pid_t pid = fork();
	if (!pid) {
		printf("i'm the child, my pid is %d\n", getpid());
		raise(SIGSTOP);
		for (int i = 0; i < n; i++)
			tab[i] = 4900;
		// idea: the child runs its event loop, processing user events
		// it is stopped by the parent before a redraw,
		// and the parent overwrites the necessary variables of the
		// child so that it behaves well when restarted
		return 33;
	} else {
		printf("i'm the parent of %d, my pid is %d!\n", pid, getpid());
		//kill(pid, SIGSTOP);
		getchar();
		kill(pid, SIGCONT);
		int rv;
		waitpid(pid, &rv, 0);
		printf("my child returned with value %d (%d %d)\n", rv,
				WEXITSTATUS(rv), WIFEXITED(rv)
				);
		int idx = rand()%n;
		printf("the magic number[%d] was %g\n", idx, tab[idx]);
	}
	free_shareable_memory(tab, n);
	printf("i'm the parent again\n");
	return 0;
}
