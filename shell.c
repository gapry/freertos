#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help")
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){

}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if(fd==OPENFAIL)
		return 0;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
	}

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
	fio_printf(1, "\r\n%s\r\n", buf);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

	if(!filedump(argv[1]))
		fio_printf(2, "\r\n%s no such file or directory.\r\n", argv[1]);
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

	if(!filedump(buf))
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command (int ipt_len, char *argv[]) 
{
	if (ipt_len > 1) 
	{
		int rnt = 0, i = 0, cmd_len = 0;
		char ipt[50] = {'\0'}, cmd_buf[10] = {'\0'}, arg_buf[10] = {'\0'};
		char* space = " ";
		char* cmd_write = "write";
		char* w_file = ">> sysinfo";
		ipt_len = strlen (*(argv + 1));
		
		strcpy (arg_buf, 2[argv]);
		if (strcmp (1[argv], cmd_write) == 0)
			strcpy (cmd_buf, "echo");
		else	
			strcpy (cmd_buf, 1[argv]);

		for (i = 0; i < strlen (cmd_buf); ++i) 
			(ipt + i)[0] = i[&cmd_buf[0]];

		cmd_len = i;
		*(ipt + cmd_len) = *space;
		for (i = 0; i < strlen (arg_buf); ++i) 
			*(ipt + i + cmd_len + 1) = arg_buf[i];
		//fio_printf (1, "\r\n %s .\r\n", ipt);

		cmd_len = strlen (cmd_buf) + 1 + strlen (arg_buf);
		*(ipt + cmd_len) = *space;
		for (i = 0; i < strlen (w_file) && strcmp (*(argv + 1), cmd_write) == 0; ++i)
			*(ipt + i + cmd_len + 1) = *(w_file + i);
		//fio_printf (1, "\r\n %s .\r\n", ipt);
		
		if (argv[1][0] == '\'') 
		{
			argv[1][ipt_len - 1] = '\0';
			rnt = host_system (ipt);
		} else {
			rnt = host_system (ipt);
		}

		fio_printf (1, "\r\nfinish with exit code %d.\r\n", rnt);
	} else
		fio_printf (2, "\r\nUsage: host 'command'\r\n");
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i=0;i<sizeof(cl)/sizeof(cl[0]); ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
