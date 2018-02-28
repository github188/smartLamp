#include "config.h"

int fgetstr( FILE *fp, char *str )
{
    char    tmp[1000];

    fgets( tmp, 999, fp );
    if ( !strlen( tmp ) )
        return 1;
    tmp[strlen( tmp ) - 1] = 0;
    strcpy( str, tmp );

    return 0;
}


int cutpre( char *str )
{
    int     i   = 0,len;
    char    tmp[999];
    while ( str[i] == ' ' || str[i] == '\t' )
        i++;
    sprintf( tmp, "%s", str + i );
    sprintf( str, "%s", tmp );
    return( 0 );
}


char * strlwr( char *mstr )
{
    char    *pchar;

    pchar = mstr;
    while ( *mstr != 0 )
    {
        if ( isupper( *mstr ) )
        {
            /*uppercase */
            *mstr = *mstr + 32;
        }
        mstr++;
    }
    mstr = pchar;
    return ( char * ) mstr;
}

int cuttail( char *str )
{
    int     i   = 0,len;
    char    tmp[999];
    sprintf( tmp, "%s", str );
    i = 0;
    while ( tmp[i] != '\0' && tmp[i] != '\t' && tmp[i] != ';' )
        i++;
    tmp[i] = 0;
    sprintf( str, "%s", tmp );
    return( 0 );
}


int killblk( char *str )
{
    int     i   = 0,len;
    char    tmp[999];
    while ( str[i] == ' ' )
        i++;
    sprintf( tmp, "%s", str + i );
    i = 0;
    while ( tmp[i] != ' '
         && tmp[i] != '\0'
         && tmp[i] != '\t'
         && tmp[i] != '\r' )
        i++;
    tmp[i] = 0;
    sprintf( str, "%s", tmp );
    return( 0 );
}

// 获取整数类型的配置项值
int GetPrivateProfileInt( char *AppName
                        , char *KeyName
                        , int def
                        , char *FileName )
{
    FILE    *fp;
    char    *p;
    char    ConfFile[MAX_PATH];

    char    tmp[500];
    char    AppPat[100];
    char    KeyName1[100];

    strcpy( KeyName1, KeyName );
	/*
    if ( ( p = getenv( WORK_DIR ) ) == NULL )
    {
        sprintf( ConfFile, "%s/etc/%s", WORK_DIR_DEFINE,FileName );
    }
    else
    {
        sprintf( ConfFile, "%s/etc/%s", p, FileName );   // 将配置文件放到etc目录下
    }
    */

	sprintf( ConfFile, "%s/etc/%s", WORK_DIR_DEFINE,FileName );

    sprintf( AppPat, "[%s]", AppName );
    fp = fopen( ConfFile, "r" );
    if ( fp == NULL )
        return def;
    while ( !feof( fp ) )
    {
        memset( tmp, 0, 500 );
        fgetstr( fp, tmp );
        if ( tmp[0] == '*'
          || tmp[0] == ';'
          || tmp[0] == 0
          || tmp[0] == '\n'
          || tmp[0] == '/' )
            continue;
        cutpre( tmp );
        if ( strncmp( strlwr( AppPat ), strlwr( tmp ), strlen( AppName ) + 2 )
          == 0 )
        {
            while ( !feof( fp ) )
            {
                memset( tmp, 0, 500 );
                fgetstr( fp, tmp );
                if ( tmp[0] == '*'
                  || tmp[0] == ';'
                  || tmp[0] == 0
                  || tmp[0] == '\n'
                  || tmp[0] == '/' )
                    continue;
                cutpre( tmp );
                if ( strncmp( strlwr( KeyName1 )
                            , strlwr( tmp )
                            , strlen( KeyName ) ) == 0 )
                {
                    int     i   = 0;
                    char    tmpint[500];

					/*lint -e722*/
                    while ( tmp[i++] != '=' );
					/*lint +e722*/
                    sprintf( tmpint, "%s", tmp + i );
                    cuttail( tmpint );
                    fclose( fp );
                    return( atoi( tmpint ) );
                }
                else if ( tmp[0] == '[' )
                {
                    fclose( fp );
                    return( def );
                }
                else
                    ;
            }
        }
    }
    fclose( fp );
	fp = NULL;
	
    return( def );
}


// 获取字符串类型的配置项值
int GetPrivateProfileString( char *AppName
                           , char *KeyName
                           , char *def
                           , char *out
                           , int len
                           , char *FileName )
{
    FILE    *fp;
    char    *p;
    char    ConfFile[MAX_PATH];

    char    tmp[500],tmp1[500];
    char    AppPat[100];
    char    KeyName1[100];

    strcpy( KeyName1, KeyName );


    /*
      if ( ( p = getenv( WORK_DIR ) ) == NULL )
      {
           sprintf( ConfFile, "%s/etc/%s", WORK_DIR_DEFINE,FileName );
      }
      else
      {
          sprintf( ConfFile, "%s/etc/%s", p, FileName );    // 将配置文件放到etc目录下
      }
      */
    sprintf( ConfFile, "%s/etc/%s", WORK_DIR_DEFINE,FileName );

    sprintf( AppPat, "[%s]", AppName );
    fp = fopen( ConfFile, "r" );
    if ( fp == NULL )
        return -1;
    while ( !feof( fp ) )
    {
        memset( tmp, 0, 500 );
        fgetstr( fp, tmp );
        if ( tmp[0] == '*'
          || tmp[0] == ';'
          || tmp[0] == 0
          || tmp[0] == '\n'
          || tmp[0] == '/' )
            continue;
        cutpre( tmp );
        if ( strncmp( strlwr( AppPat ), strlwr( tmp ), strlen( AppName ) + 2 )
          == 0 )
        {
            while ( !feof( fp ) )
            {
                memset( tmp, 0, 500 );
                fgetstr( fp, tmp );
                if ( tmp[0] == '*'
                  || tmp[0] == ';'
                  || tmp[0] == 0
                  || tmp[0] == '\n'
                  || tmp[0] == '/' )
                    continue;
                cutpre( tmp );
                memcpy( tmp1, tmp, 500 );
                if ( strncmp( strlwr( KeyName1 )
                            , strlwr( tmp1 )
                            , strlen( KeyName ) ) == 0 )
                {
                    int     i   = 0;
                    char    tmpch[500];

                    while ( tmp[i++] != '=' );
					
                    sprintf( tmpch, "%s", tmp + i );
                    killblk( tmpch );
                    sprintf( out, "%s", tmpch );
                    fclose( fp );
                    return( 0 );
                }
                else if ( tmp[0] == '[' )
                {
                    strcpy( out, def );
                    fclose( fp );
                    return( 0 );
                }
            }
        }
    }
    strcpy( out, def );
    fclose( fp );
	fp = NULL;
	
    return( 0 );
}



