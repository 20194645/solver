Hướng dẫn cài đặt:
1. Cài đặt SCIP theo link: https://www.scipopt.org/
2. Thay đường dẫn của file example.txt ở trong file reader.hpp
3. Chỉnh lại đường dẫn của scip trong Makefile rồi gõ lệnh make trên terminal
4. Sau khi biên dịch xong thì chạy main ở thư mục bin 


Lưu ý: Cài đặt solver trên môi trường linux hoặc wsl2 


Lưu ý 2:
     Đã có thêm chức năng đọc file trực tiếp từ TSG.txt.
     
     Khi gọi gõ ./main truyền 1 số bất kỳ ở đằng sau như ./main 1 để đọc từ TSG.txt
     
     TSG.txt cần chỉnh sửa 1 thêm các dòng: 
     
     - c, n , đích trong không gian , flow , earliness, tardliness , điểm đích trong đồ thị TSG 
     
     - c N  Số đỉnh trong không gian 
     
     - alpha 1 // Các hệ số 
     
     - beta 1  // Các hệ số 
     
     - gamma 1 // Các hệ số
     
     Khi chạy trong giả lập cũng cần phải thay đổi code trong giả lập để mỗi khi cập nhật TSG.txt thì nó cũng phải in ra thêm các dòng có định dạng như này 
     
