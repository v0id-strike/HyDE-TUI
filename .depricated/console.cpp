#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <asm-generic/ioctls.h>

void getTerminal_Size (int &rows, int&cols) {
    struct
        winsize
        window_Size
    ; // struct defining terminal window size
    
    if
        ( ioctl ( STDOUT_FILENO , TIOCGWINSZ , &window_Size ) == -1 )
    {
        
        perror
            ( "ioctl" )
        ;
        
        rows = 0;
        cols = 0;
        
    }
    
    else
    {
        
        rows =
            window_Size.ws_row
        ;
        
        cols =
            window_Size.ws_col
        ;
        
    }
    
}

int
    rows { } ,
    cols { }
;

void
    handle_WinCH
    ( int sig )
{
    
    ( void )
        sig
    ; // unused parameter
    
    static
      int
        s_prevRows { } ,
        s_prevCols { }
    ;
    
    static
      bool s_isPrev = false;
    
    getTerminal_Size
        ( rows , cols )
    ;
    
    if
        ( rows <= 35 && cols <= 50 )
    {
        
        system("clear");
        
        for
            ( int i { }; i < ( cols - 3 ) / 2; ++i )
            std::cerr
                <<  "*"
            ;
            
            std::cerr
                <<  ( cols - 3 ) / 2
                <<  ( rows - 3 ) / 2
            ;
        
        
        // std::cerr
        //     <<  "[ERROR]: Terminal size is too small!\n"
        //     <<  "Please resize the terminal.\n"
        //     <<  "\033[0;90m"
        //     <<  rows
        //     <<  " , "
        //     <<  "\033[0;60m"
        //     <<  cols
        //     <<  '\n'
        // ;
        
    }
    
    else
    {
        
        system ( "clear" );
        
        // if
        //     ( s_isPrev )
        //     std::cerr
        //         <<  "\r Previous size => "
        //         <<  "rows: "
        //         <<  s_prevRows
        //         <<  " , "
        //         <<  "columns: "
        //         <<  s_prevCols
        //         <<  '\n'
        //     ;
        
        // std::cerr
        //     <<  "New size => "
        //     <<  "rows: "
        //     <<  rows
        //     <<  " , "
        //     <<  "columns: "
        //     <<  cols
        //     <<  '\n'
        // ;
        
        // s_isPrev = true;
        
        // s_prevRows = rows;
        // s_prevCols = cols;
        
        std::cerr
            <<  "╭"
        ;
        
        for
            ( int i { }; i < cols - 2; ++i )
        {
            
            std::cerr
                <<  "─"
            ;
            
        }
        
        std::cerr
            <<  "╮\n"
        ;
        
        for
            ( int i { }; i < rows - 4; ++i )
        {
            
            std::cerr
                <<  "│"
            ;
            
            for
                ( int j { }; j < cols - 2 ; ++j )
            {
                
                std::cerr
                    <<  "\033[40m "
                ;
                
            }
            
            std::cerr
                <<  "\033[0m|\n"
            ;
            
        }
        
        std::cerr
            <<  "╰"
        ;
        
        for
            ( int i { }; i < cols - 2; ++i )
        {
            
            std::cerr
                <<  "─"
            ;
            
        }
        
        std::cout
            <<  "╯"
        ;
        
        std::cerr
            <<  ">>"
            <<  ' '
        ;
        
    }
    
}

static bool s_isExit = false;

void
    get_Win_Term_Size
    ( )
{
    
    using
        namespace
        std::chrono_literals
    ;
    
    while
        ( !s_isExit )
    {
        
        signal
            ( SIGWINCH , handle_WinCH )
        ;
        
        std::this_thread::sleep_for( 1s );
        
    }
    
}


int
    main
    ( void )
{
    
    std::thread winSize_Worker ( get_Win_Term_Size );
    
    getTerminal_Size
        ( rows , cols )
    ;
    
    std::cout
        <<  "Initial Terminal size: "
        <<  rows
        <<  " rows , "
        <<  cols
        <<  " columns"
        << '\n'
    ;
    
    while
        ( !s_isExit )
        if
            ( std::getchar ( ) == 'q' )
            s_isExit = true;
    
    winSize_Worker.join();
    
    std::cout
        <<  "Exited successfully~"
    ;
    
}
