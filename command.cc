
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();
    for (size_t i = 0; i< tokens.size();i++){
	 if (tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT)
         {
         currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
       	}
	else if (tokens[i].type == TOKEN_REDIRECT)
	{ 
		if (tokens[i+1].type != TOKEN_ARGUMENT)
		{ printf ("error file name missing") ;
	        	return;}
		currentCommand._outFile= strdup(tokens[i+1].value.c_str());
		i++;
	}
	else if (tokens[i].type == TOKEN_APPEND)
        { 
                if (tokens[i+1].type != TOKEN_ARGUMENT)
                { printf ("error file name missing") ;
                        return;}
                currentCommand._outFile= strdup(tokens[i+1].value.c_str());
                currentCommand._append=1;
		 i++;
        }
	else if (tokens[i].type == TOKEN_INPUT)
        { 
                if (tokens[i+1].type != TOKEN_ARGUMENT)
                { printf ("error file name missing") ;
                        return;}
                currentCommand._inputFile= strdup(tokens[i+1].value.c_str());
                i++;
        }


	else if (tokens[i].type == TOKEN_ERROR)
        { 
                if (tokens[i+1].type != TOKEN_ARGUMENT)
                { printf ("error file name missing") ;
                        return;}
                currentCommand._errFile= strdup(tokens[i+1].value.c_str());
                i++;
        }
	else if (tokens[i].type == TOKEN_REDIRECT_AND_ERROR)
        { 
                if (tokens[i+1].type != TOKEN_ARGUMENT)
                { printf ("error file name missing") ;
                        return;}
                currentCommand._outFile= strdup(tokens[i+1].value.c_str());
		currentCommand._errFile= strdup(tokens[i+1].value.c_str());
		currentCommand._append=1;
		currentCommand._out_error=1;
                i++;
        }
	else if (tokens[i].type == TOKEN_PIPE)
        { 
                if (currentSimpleCommand->_numberOfArguments==0)
                { printf ("error nothing before pipe") ;
                        return;}
                currentCommand.insertSimpleCommand(currentSimpleCommand);
                currentSimpleCommand =new SimpleCommand();
        }
	else if (tokens[i].type == TOKEN_BACKGROUND)
	{
		currentCommand._background =1;
	}

   }
   if (currentSimpleCommand->_numberOfArguments ==0){
	printf("error empty command");
	return;
	}
    currentCommand.insertSimpleCommand(currentSimpleCommand);
    currentCommand.execute();
    /*
      Implement this function 
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    if (_numberOfSimpleCommands == 0)
    {
	clear();
	return;
    }
    char **first = _simpleCommands[0]->_arguments;

    if (_numberOfSimpleCommands == 1 && strcmp(first[0], "exit") == 0)
    {
        printf("Good bye!!\n");
        clear();
        exit(0);
    }

    if (_numberOfSimpleCommands == 1 && strcmp(first[0], "cd") == 0)
    {
        const char *dir = first[1] ? first[1] : getenv("HOME");
        if (dir == NULL || chdir(dir) != 0)
            perror("cd");
        clear();
        return;
    }
    int oldin = dup(0);
    int oldout =dup(1);
    int olderr =dup(2);
    
    int fin;
    if (_inputFile)
    {
	fin = open(_inputFile,O_RDONLY);
	if (fin <0 )
	{
		perror(_inputFile);
		close(oldin);
		close(oldout);
		close(olderr);
		clear();
		return;
         }
     }
	else
		fin = dup(oldin);

     int fout =-1;
     if (_outFile)
     {
	int flags  = O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC );
	fout = open(_outFile, flags, 0644);
	if (fout <0 )
        {
                perror(_outFile);
                close(oldin);
                close(oldout);
                close(olderr);
		close(fin);
                clear();
                return;
         }
     }

     int ferr ;
     if (_out_error && fout >= 0)
	ferr = dup(fout);
     else if(_errFile)
     {
	ferr = open(_errFile,O_WRONLY |O_CREAT |O_TRUNC ,0644);
	if (ferr <0 )
        {
                perror(_errFile);
                close(oldin);
                close(oldout);
                close(olderr);
		close(fin);
		if (fout >=0)
		   close(fout);
                clear();
                return;
         }
     }
     else
	ferr = dup (olderr);

    int pid = 0;
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        dup2(fin, 0);
        close(fin);

        int Ofile;
        if (i == _numberOfSimpleCommands - 1)
       {
            if (fout >= 0){
                Ofile = fout;}
            else{
                Ofile = dup(oldout);}
        }
        else
        {
            int fdpipe[2];
            pipe(fdpipe);
            Ofile = fdpipe[1];
            fin= fdpipe[0];
        }

        dup2 ( Ofile, 1);
        close ( Ofile);
        dup2 ( ferr, 2);

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            break;
        } 
	if (pid == 0)
        {
            close(ferr);
	    close(oldin);
	    close(oldout);
	    close(olderr);
            char **args = _simpleCommands[i]->_arguments;
	    signal(SIGINT,SIG_DFL);
            execvp(args[0], args);
            perror(args[0]);
            _exit(1);
        }
    }

    dup2(oldin, 0);
    dup2(oldout, 1);
    dup2(olderr, 2);
    close(oldin);
    close(oldout);
    close(olderr);
    close(ferr);

    if (!_background)
        waitpid(pid, NULL, 0);



    /*
        Write your code here
    */

    clear();
}

void Command::prompt()

{
    std::string input;
    while (true)
    {   printf("myshell>");
        fflush(stdout);
        if(!std::getline(std::cin, input)){
		break; }
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

void handleChild(int sig)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        FILE *f = fopen("shell.log", "a");
        if (f)
        {
            fprintf(f, "Child %d terminated\n", pid);
            fclose(f);
        }
    }
}

int main()
{   
   signal(SIGINT, SIG_IGN);

    struct sigaction sa;
    sa.sa_handler = handleChild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL); 

    Command::_currentCommand.prompt();
    return 0;
}
