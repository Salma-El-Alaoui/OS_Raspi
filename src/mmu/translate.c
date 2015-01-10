unsigned int
translate(unsigned int va)
{
  unsigned int pa; /* The result */

  /* 1st and 2nd table addresses */
  unsigned int table_base;
  unsigned int second_level_table;

  /* Indexes */
  unsigned int first_level_index;
  unsigned int second_level_index;
  unsigned int page_index;
  
  /* Descriptors */
  unsigned int first_level_descriptor;
  unsigned int* first_level_descriptor_address;
  unsigned int second_level_descriptor;
  unsigned int* second_level_descriptor_address;

  __asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
  
  table_base = table_base & 0xFFFFC000;
  
  /* Indexes*/
  first_level_index = (va >> 20);
  second_level_index = ((va << 12) >> 24);
  page_index = (va & 0x00000FFF);

  /* First level descriptor */
  first_level_descriptor_address = (unsigned int*) (table_base | (first_level_index << 2));
  first_level_descriptor = *(first_level_descriptor_address);

  /* Second level descriptor */
  second_level_table = first_level_descriptor & 0xFFFFFC00;
  second_level_descriptor_address = (unsigned int*) (second_level_table | (second_level_index << 2));
  second_level_descriptor = *((unsigned int*) second_level_descriptor_address);    

  /* Physical address */
  pa = (second_level_descriptor & 0xFFFFF000) | page_index;

  return pa;
}
