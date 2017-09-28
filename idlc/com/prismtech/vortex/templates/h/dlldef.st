dlldef (name, dllname, dllfile) ::= <<

<if(dllfile)>#include "<dllfile>"<else>
#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #ifdef _<name>_DLL_
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #ifdef DDS_BUILD_<dllname>_DLL
      #define DDS_EXPORT extern
    #else
      #define DDS_EXPORT extern __declspec (dllimport)
    #endif
  #endif
#else
  #define DDS_EXPORT extern
#endif
<endif>
>>
