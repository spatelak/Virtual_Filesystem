#include "simpleFS.h"

void test_mkdir() {
  printf("CALL 1:\n");
  make_directory("/Testdir1", 9);

  printf("\n\nCALL 2:\n");
  make_directory("/Testdir1/abc", 13);

  //printf("\n\nCALL 3\n");
  //make_directory("/Testdir1/", 10);
}

void test_rddir() {
  printf("\n------------\n");
  printf("\n\nCALL 1:\n");
  char *data1 = (char *) malloc(BLOCK_SIZE);
  int len1 = read_directory("/", 1, data1);
  printf("data1 length = %d\n", len1);

  printf("\n\nCALL 2:\n");
  char *data2 = (char *) malloc(BLOCK_SIZE);
  int len2 = read_directory("/Testdir1/abc", 13, data2);
  printf("data2 length = %d\n\n\n", len2);
}

void test_rmdir() {
  rm_directory("/Testdir1", 9);
  rm_directory("/Testdir1/abc", 13);
  rm_directory("/Testdir1", 9);

  test_rddir();
}

void test_mkfile() {
  printf("\n\n\n");
  create_file("/rFile1", strlen("/rFile1"), 5, "hello");
}

void test_rdfile() {
  printf("\n\nREAD FILE\n\n");
  char *data = (char *) malloc(6);
  int len3 = read_file("/rFile1", strlen("/rFile1"), data);
  printf("len3 = %d\n", len3);
}

void test_rmfile() {
  rm_file("/rFile1", strlen("/rFile1"));
}

int main(int argc, char **argv)
{
  /* TODO:
   * Implement :)
   */

  /*
  //init_filesystem(20, "./filesystem", strlen("./filesystem"));
  //open_filesystem("./filesystem", strlen("./filesystem"));
  */
  
  //char *data = "et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit cursus nunc, quis gravida magna mi a libero. Fusce vulputate eleifend sapien. Vestibulum purus quam, scelerisque ut, mollis sed, nonummy id, metus. Nullam accumsan lorem in dui. Cras ultricies mi eu turpis hendrerit fringilla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In ac dui quis mi consectetuer lacinia. Nam pretium turpis et arcu. Duis arcu tortor, suscipit eget, imperdiet nec, imperdiet iaculis, ipsum. Sed aliquam ultrices mauris. Integer ante arcu, accumsan a, consectetuer eget, posuere ut, mauris. Praesent adipiscing. Phasellus ullamcorper ipsum rutrum nunc. Nunc nonummy metus. Vestibulum volutpat pretium libero. Cras id dui. Aenean ut eros et nisl sagittis vestibulum. Nullam nulla eros, ultricies sit amet, nonummy id, imperdiet feugiat, pede. Sed lectus. Donec mollis hendrerit risus. Phasellus nec sem in justo pellentesque facilisis. Etiam imperdiet imperdiet orci. Nunc nec neque. Phasellus leo dolor, tempus non, auctor et, hendrerit quis, nisi. Curabitur ligula sapien, tincidunt non, euismod vitae, posuere imperdiet, leo. Maecenas malesuada. Praesent congue erat at massa. Sed cursus turpis vitae tortor. Donec posuere vulputate arcu. Phasellus accumsan cursus velit. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed aliquam, nisi quis porttitor congue, elit erat euismod orci, ac placerat dolor lectus quis orci. Phasellus consectetuer vestibulum elit. Aenean tellus metus, bibendum sed, posuere ac, mattis non, nunc. Vestibulum fringilla pede sit amet augue. In turpis. Pellentesque posuere. Praesent turpis. Aenean posuere, tortor sed cursus feugiat, nunc augue blandit nunc, eu sollicitudin urna dolor sagittis lacus. Donec elit libero, sodales nec, volutpat a, suscipit non, turpis. Nullam sagittis. Suspendisse pulvinar, augue ac venenatis condimentum, sem libero volutpat nibh, nec pellentesque velit pede quis nunc. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Fusce id purus. Ut varius tincidunt libero. Phasellus dolor. Maecenas vestibulum mollis diam. Pellentesque ut neque. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. In dui magna, posuere eget, vestibulum et, tempor auctor, justo. In ac felis quis tortor malesuada pretium. Pellentesque auctor neque nec urna. Proin sapien ipsum, porta a, auctor quis, euismod ut, mi. Aenean viverra rhoncus pede. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Ut non enim eleifend felis pretium feugiat. Vivamus quis mi. Phasellus a est. Phasellus magna. In hac habitasse platea dictumst. Curabitur at lacus ac velit ornare lobortis.Cura";

  //char *data1 = (char *) malloc(strlen(data) + 1);
  //data1 = "/0";
  init_filesystem(20, "../filesystemImage", strlen("../filesystemImage"));
  //create_file("/a", strlen("/a"), strlen(data), data);
  //create_file("/a", strlen("/a"), 0, data1);
  //make_directory("/a", strlen("/a"));
  //create_file("/A/a", strlen("/A/a"), strlen(data), data);
  //make_directory("/c", strlen("/c"));
  //make_link("/A/a", strlen("/A/a"), "/b");
  //make_link("/b", strlen("/b"), "/A/a");
  //read_file("/b", strlen("/b"), data1);
  //if (strcmp(data, data1) == 0) printf("PASSED\n");
  //sleep(10);
  
  //make_directory("/b", strlen("/b"));
  //make_directory("/c", strlen("/c"));
  //rm_directory("/b", strlen("/b"));
  //make_directory("/d", strlen("/d"));
  //make_directory("/e", strlen("/e"));
  //make_directory("/f", strlen("/f"));
  //read_directory("/", strlen("/"), data1);
  //printf("%d\n", strlen(data));
  /*
  //create_file("/f1/", strlen("/f1/"), strlen(data), data);

  make_directory("/Child dir", strlen("/Child dir"));
  make_directory("/Child dir/Grandson/", strlen("/Child dir/Grandson/"));
  create_file("/Child dir/Grandson/omnia dicta fortiora si dicta Latina", strlen("/Child dir/Grandson/omnia dicta fortiora si dicta Latina"), strlen(data), data);
  make_link("/Child dir/Link", strlen("/Child dir/Link"), "/Child dir/Grandson/omnia dicta fortiora si dicta Latina");

  //open_filesystem("./TestImages/simpleFS_multiple_operations", strlen("./TestImages/simpleFS_multiple_operations"));
  
  char *data1 = (char *) malloc(4100);
  //read_file("/Child dir/Grandson/omnia dicta fortiora si dicta Latina", strlen("/Child dir/Grandson/omnia dicta fortiora si dicta Latina"), data1);

  //char *data2 = (char *) malloc(4100);
  //read_file("/Child dir/Link", strlen("/Child dir/Link"), data2);
  
  //if(strcmp(data1, data2) == 0) printf("PASSED1\n");

  //rm_file("/Child dir/Link", strlen("/Child dir/Link"));
  rm_file("/Child dir/Grandson/omnia dicta fortiora si dicta Latina", strlen("/Child dir/Grandson/omnia dicta fortiora si dicta Latina"));
  
  char *data3 = (char *) malloc(4100);
  //read_file("/Child dir/Grandson/omnia dicta fortiora si dicta Latina", strlen("/Child dir/Grandson/omnia dicta fortiora si dicta Latina"), data3);
  read_file("/Child dir/Link", strlen("/Child dir/Link"), data3);
  
  if (strcmp(data, data3) == 0) printf("PASSED2\n");
  //printf("%s\n", data1);
  
  
  test_mkdir();
  test_rddir();
  //test_rmdir();

  test_mkfile();
  test_rdfile();

  test_rddir();

  test_rmfile();
  //make_directory("/Child dir", strlen("/Child dir"));
  test_rddir();
  */
  return 0;
}

