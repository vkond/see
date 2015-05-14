#define MEM_
#include "see.hpp"

/* ��� ������� �������� ����� ��������� ������������ ������
*/

unsigned long getAvailableMemory () {

 unsigned long volume;

// ���� � ������� ���������� ��� �������, �� �������� ����� ���
#ifdef _SC_AVPHYS_PAGES
 #ifdef _SC_PAGESIZE

  /* � Linux'� ������ ������� ����� ������������� ������ �����
     ������ ���������� �� ����������, ����� �� ������� free ��������� ������ ����� ����.
     �������, ����� ����� ������ ������ � ������ �� �� 3 (������������ �����������)
  */
#if HAVE_SYSCONF  
   volume = ( sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) ) / 3;
#endif
  return volume;

 #endif
#endif


// ���� ������� _SC_... �� ����������, �� ����� �������� ����� ��������� utsname,
// ����� ���������� ������� � �� version
// ���� ��� Linux, �� ���������� sysinfo (��� ������ ������ Linux
// sysinfo ������ ���� ��-�������)
#ifdef __NO_MEMORY_MACROS__

#if HAVE_SYS_UTSNAME_H
struct utsname utsinfo;

# if HAVE_UNAME
   if (uname(&utsinfo) < 0) return 0;                   // ���� �� ������ �������� ���� - �������
# endif

#endif

//if (strcasecmp(utsinfo.sysname, "Linux")) return 0;  // ���� ��� �� Linux - �������
#ifdef __YES_LINUX__

// �������� ������ ����
// ���� ������ 2.3.16 �� sysinfo ���������� ���� � ������
// ���� ������ 2.3.23 (i386) ��� 2.3.48 (all architectures) �� ���������� � ������������ ������
//int a, b, c;
//sscanf(utsinfo.release, "%d.%d.%d", &a, &b, &c);
//unsigned long release = 65536*a + 256*b + c;

// ����������, ����� ��� ������
bool intel = false;
#if HAVE_SYS_UTSNAME_H
  if (utsinfo.machine[0] == 'i' && utsinfo.machine[2] == '8' && utsinfo.machine[3] == '6') intel = true;
#endif

#if HAVE_SYS_SYSINFO_H
struct sysinfo sinfo;

#if HAVE_SYSINFO

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,3,16)
//if (release <= 2*65536 + 3*256 + 16) {
 if (!sysinfo(&sinfo)) { volume = sinfo.totalram / 3; return volume; }
  else return 0;
//}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,23)
 #if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,48)
//if (release >= 2*65536 + 3*256 + 48 ||
//    (release >= 2*65536 + 3*256 + 23 && intel) ) {
 if (intel) if (!sysinfo(&sinfo)) { volume = ( sinfo.totalram * sinfo.mem_unit ) / 3; return volume; }
             else return 0;	     
//}
 #endif
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,48)
 if (!sysinfo(&sinfo)) { volume = ( sinfo.totalram * sinfo.mem_unit ) / 3; return volume; }
  else return 0;  
#endif

#endif //#if HAVE_SYSINFO

#endif //#if HAVE_SYS_SYSINFO_H 

#endif //#ifdef __YES_LINUX__

#endif //#ifdef __NO_MEMORY_MACROS__

return 0;
}


