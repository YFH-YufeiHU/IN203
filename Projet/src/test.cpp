    #include<iostream>
    #include<vector>
    // #include"omp.h"
     
    using namespace std;
     
    int main()
    {
    //gcc program-source-code.c -o executable-file-name
    // #pragma omp parallel for num_threads(8)
    // 	for (int i = 0; i < 12; i++)
    // 	{
    // 		printf("i= %d:OpenMP Test, 线程编号为: %d\n", i,omp_get_thread_num());
            
    // 	}
    // 	system("pause");

    //     return 0;
        // std::vector<vector<char>> send_buff(2);
        // send_buff[0]
        // std::cout<<send_buff[0].data()<<endl;
        for(int k=2;k<4;++k){
            std::cout<<"loc_height ==>"<<k<<std::endl;
        }
        return 0;
    }