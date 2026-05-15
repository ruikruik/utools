SECTIONS
{
  .data :
  {
    _rtable_start = . ;
    __rtable_start = . ;
    *(.rtable)
    _rtable_end = . ;
    __rtable_end = . ;
  }
}
INSERT AFTER .text;
