#ifndef __STUB_H__
#define __STUB_H__

#ifdef __cplusplus
extern "C" {
#endif
    int uninstall_stub(void* stub_f);
    int install_stub(void *orig_f, void *stub_f, char *desc);
 
#define INSTALL_STUB(o,s) install_stub((void*)o,(void*)s,(char*)#o"->"#s)
#define REMOVE_STUB(s) uninstall_stub((void*)s)
 
#ifdef __cplusplus
}
#endif

#endif  // __STUB_H__