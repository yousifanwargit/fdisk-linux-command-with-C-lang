#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
typedef struct{
    uint8_t status;
    uint8_t first_chs[3];
    uint8_t partition_type;
    uint8_t last_chs[3];
    uint32_t lba;
    uint32_t sector_count;
}partitionEntry;
typedef struct{
    uint8_t status;
    uint8_t first_chs[3];
    uint8_t partition_type;
    uint8_t last_chs[3];
    uint32_t lba;
    uint32_t sector_count;
}ebr_entry;

int main(int argc, char **argv){

    char buf[512];
    char *myhard = argv[1];
    char *partition_type[4];
    char *logical_partition_type[4];
    int fd = open(myhard, O_RDONLY);
    read(fd, buf, 512);

    partitionEntry *table_entry_ptr = (partitionEntry *) & buf[446];

    partitionEntry table_entry_arr[4];

    printf("%-10s |%-10s |%-10s |%-10s |%-10s |%-10s |%-10s |%-10s\n",
            "Device","Boot","Start","End","Sectors","Size","Id","Type");
    
    for(int i=0; i<4 ; i++){
        
        if(table_entry_ptr[i].partition_type == 0x83){
            partition_type[i] = "Linux";
        }
        else if(table_entry_ptr[i].partition_type == 0x05){
            partition_type[i] = "Extended";
        }
        else if(table_entry_ptr[i].partition_type == 0x07){
            partition_type[i] = "NTFS";
        }
        else if(table_entry_ptr[i].partition_type == 0x0C || table_entry_ptr[i].partition_type == 0x0B){
            partition_type[i] = "FAT32";
        }
        else{
            partition_type[i] = "non";
        }
        
        if(table_entry_ptr[i].sector_count == 0){
            continue;
        }
        
        printf("%s%d  |%-10c |%-10u |%-10u |%-10u |%-9uG |%-10X |%-10s\n",
            argv[1],
            i+1,
            table_entry_ptr[i].status == 0x80 ? '*' : ' ',
            table_entry_ptr[i].lba,
            table_entry_ptr[i].lba + table_entry_ptr[i].sector_count - 1,
            table_entry_ptr[i].sector_count,
            (uint32_t)((((uint64_t)table_entry_ptr[i].sector_count * 512) / (1024 * 1024 * 1024))),
            table_entry_ptr[i].partition_type,
            partition_type[i]);

            table_entry_arr[i] = table_entry_ptr[i];
            //printf("%-10d\n", table_entry_arr[i].lba);
    }
    int new_seek = lseek(fd, 0, SEEK_SET);
    int num_of_logical_partition = 4;

    for(int i=0; i<4 ; i++){
        
        uint32_t offset_of_ebr = 0;
        ebr_entry *EBR_ptr;
        if(!(strcmp(partition_type[i], "Extended"))){

            do{
            new_seek = lseek(fd, (offset_of_ebr + table_entry_arr[i].lba)*512, SEEK_SET);
            int read_status = read(fd, buf, 512);
            ebr_entry *EBR_ptr = (ebr_entry *) & buf[446];
            uint32_t sector_count = EBR_ptr[0].sector_count;
            char *type_of_logical_partition;
            if(EBR_ptr[0].partition_type == 0x83){
                type_of_logical_partition = "Linux";
            }
            else if(EBR_ptr[0].partition_type == 0x05){
                type_of_logical_partition = "Extended";
            }
            else if(EBR_ptr[0].partition_type == 0x07){
                type_of_logical_partition = "NTFS";
            }
            else if(EBR_ptr[0].partition_type == 0x0C || EBR_ptr[0].partition_type == 0x0B){
                type_of_logical_partition = "FAT32";
            }
            else{
                type_of_logical_partition = "non";
            }
            printf("%s%d  |%-10c |%-10u |%-10u |%-10u |%-9uM |%-10X |%-10s\n",
            argv[1],
            ++num_of_logical_partition,
            EBR_ptr[0].status == 0x80 ? '*' : ' ',
            table_entry_arr[i].lba + EBR_ptr[0].lba + offset_of_ebr,
            table_entry_arr[i].lba + EBR_ptr[0].lba + offset_of_ebr + EBR_ptr[0].sector_count - 1,
            sector_count,
            ((uint32_t)((((uint64_t)sector_count * 512) / (1024 * 1024)))),
            EBR_ptr[0].partition_type,
            type_of_logical_partition);
            offset_of_ebr = EBR_ptr[1].lba;
            }
            while(offset_of_ebr);

        }
        
    }
    return 0;
}