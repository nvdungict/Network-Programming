-- Create table if not exists (Updated with round_id)
CREATE TABLE IF NOT EXISTS questions (
  id TEXT PRIMARY KEY,
  text TEXT,
  opt_a TEXT, opt_b TEXT, opt_c TEXT, opt_d TEXT,
  correct_ans TEXT,
  round_id INTEGER DEFAULT 1
);

-- Clear old data
DELETE FROM questions;

-- Round 1: MCQ (Speed) - 30 Questions
BEGIN TRANSACTION;
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_001', 'CPU la viet tat cua chu gi?', 'Central Processing Unit', 'Central Pretty Unit', 'Core Processing Unit', 'Computer Personal Unit', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_002', 'Thu do cua Viet Nam la gi?', 'TP. Ho Chi Minh', 'Ha Noi', 'Da Nang', 'Hai Phong', 'B', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_003', '1 + 1 = ?', '1', '2', '3', '11', 'B', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_004', 'RAM la viet tat cua gi?', 'Random Access Memory', 'Read Access Memory', 'Rapid Access Memory', 'Real Access Memory', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_005', 'HTML la viet tat cua gi?', 'HyperText Markup Language', 'HighText Machine Language', 'HyperText Machine Language', 'HighText Markup Language', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_006', 'Python la gi?', 'Ngon ngu lap trinh', 'Loai ran', 'Ca hai deu dung', 'Mot loai virus', 'C', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_007', 'HTTP la gi?', 'HyperText Transfer Protocol', 'HighText Transfer Protocol', 'HyperText Transmission Protocol', 'HighText Transmission Protocol', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_008', 'SQL la viet tat cua gi?', 'Structured Query Language', 'Simple Query Language', 'Standard Query Language', 'System Query Language', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_009', 'IP la gi?', 'Internet Protocol', 'Internal Protocol', 'International Protocol', 'Integrated Protocol', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_010', 'DNS la gi?', 'Domain Name System', 'Data Name System', 'Digital Name System', 'Dynamic Name System', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_011', 'URL la viet tat cua gi?', 'Uniform Resource Locator', 'Universal Resource Locator', 'Unified Resource Locator', 'Unique Resource Locator', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_012', 'CSS la gi?', 'Cascading Style Sheets', 'Computer Style Sheets', 'Creative Style Sheets', 'Colorful Style Sheets', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_013', 'JSON la viet tat cua gi?', 'JavaScript Object Notation', 'Java Standard Object Notation', 'JavaScript Online Notation', 'Java Simple Object Notation', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_014', 'API la gi?', 'Application Programming Interface', 'Advanced Programming Interface', 'Automated Programming Interface', 'Application Process Interface', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_015', 'GUI la viet tat cua gi?', 'Graphical User Interface', 'General User Interface', 'Guided User Interface', 'Global User Interface', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_016', 'OS la viet tat cua gi?', 'Operating System', 'Online System', 'Open System', 'Optical System', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_017', 'LAN la gi?', 'Local Area Network', 'Large Area Network', 'Long Area Network', 'Limited Area Network', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_018', 'WAN la gi?', 'Wide Area Network', 'World Area Network', 'Wireless Area Network', 'Web Area Network', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_019', 'USB la viet tat cua gi?', 'Universal Serial Bus', 'Uniform Serial Bus', 'United Serial Bus', 'Universal System Bus', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_020', 'SSD la gi?', 'Solid State Drive', 'Super Speed Drive', 'System State Drive', 'Secure Storage Drive', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_021', 'HDD la gi?', 'Hard Disk Drive', 'High Density Drive', 'Hybrid Disk Drive', 'Heavy Duty Drive', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_022', 'GPU la viet tat cua gi?', 'Graphics Processing Unit', 'General Processing Unit', 'Graphical Performance Unit', 'Game Processing Unit', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_023', 'BIOS la gi?', 'Basic Input/Output System', 'Binary Input/Output System', 'Boot Input/Output System', 'Base Input/Output System', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_024', 'ROM la viet tat cua gi?', 'Read-Only Memory', 'Random-Only Memory', 'Rapid-Only Memory', 'Real-Only Memory', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_025', 'VPN la gi?', 'Virtual Private Network', 'Very Private Network', 'Verified Private Network', 'Visual Private Network', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_026', 'FTP la viet tat cua gi?', 'File Transfer Protocol', 'Fast Transfer Protocol', 'Folder Transfer Protocol', 'Free Transfer Protocol', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_027', 'TCP la gi?', 'Transmission Control Protocol', 'Transfer Control Protocol', 'Transport Control Protocol', 'Terminal Control Protocol', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_028', 'UDP la viet tat cua gi?', 'User Datagram Protocol', 'Universal Datagram Protocol', 'Unified Datagram Protocol', 'Unique Datagram Protocol', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_029', 'SSH la gi?', 'Secure Shell', 'System Shell', 'Safe Shell', 'Standard Shell', 'A', 1);
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans, round_id) VALUES ('q1_030', 'HTTPS la viet tat cua gi?', 'HyperText Transfer Protocol Secure', 'HighText Transfer Protocol Secure', 'HyperText Transmission Protocol Secure', 'HighText Transmission Protocol Secure', 'A', 1);
COMMIT;

-- Round 2: Text (Semantic) - 20 Questions
BEGIN TRANSACTION;
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_001', 'Ngon ngu lap trinh pho bien nhat cho AI?', 'Python', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_002', 'He dieu hanh ma nguon mo noi tieng co bieu tuong chim canh cut?', 'Linux', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_003', 'Cong ty so huu Windows?', 'Microsoft', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_004', 'Cong ty so huu iPhone?', 'Apple', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_005', 'Ngon ngu danh dau sieu van ban dung cho web?', 'HTML', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_006', 'Giao thuc chinh cua World Wide Web?', 'HTTP', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_007', 'Thiet bi phat song Wifi goi la gi?', 'Router', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_008', 'Don vi nho nhat cua du lieu may tinh (0 hoac 1)?', 'Bit', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_009', '8 bit tao thanh 1 gi?', 'Byte', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_010', 'Trinh duyet web cua Google?', 'Chrome', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_011', 'Mang xa hoi lon nhat the gioi?', 'Facebook', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_012', 'Ung dung nhan tin pho bien o Viet Nam?', 'Zalo', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_013', 'Cong cu tim kiem lon nhat the gioi?', 'Google', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_014', 'CEO cua Tesla va SpaceX?', 'Elon Musk', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_015', 'Ten cua chuot may tinh dau tien?', 'Douglas Engelbart', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_016', 'Phiem tat de Copy?', 'Ctrl+C', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_017', 'Phiem tat de Paste?', 'Ctrl+V', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_018', 'PDF la viet tat cua Portable Document ...?', 'Format', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_019', 'AI la viet tat cua gi?', 'Artificial Intelligence', 2);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q2_020', 'Thiet bi hien thi hinh anh tu may tinh?', 'Monitor', 2);
COMMIT;

-- Round 3: Estimation (Numeric) - 20 Questions
BEGIN TRANSACTION;
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_001', 'Nam phat hanh dau tien cua Windows?', '1985', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_002', 'Nam ra mat iPhone dau tien?', '2007', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_003', 'Chieu cao cua dinh Everest (met)?', '8848', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_004', 'Nam sinh cua Albert Einstein?', '1879', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_005', 'So luong xuong trong co the nguoi truong thanh?', '206', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_006', 'Toc do anh sang (km/s) gan dung?', '300000', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_007', 'Nam Ket thuc Chien tranh The gioi thu 2?', '1945', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_008', 'Dan so Viet Nam nam 2023 (trieu nguoi)?', '100', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_009', 'So luong nguyen to hoa hoc trong bang tuan hoan hien nay?', '118', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_010', 'Nam Google duoc thanh lap?', '1998', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_011', 'Nam Facebook duoc thanh lap?', '2004', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_012', 'Khoang cach tu Trai Dat den Mat Trang (nghin km)?', '384', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_013', 'Nhiet do soi cua nuoc o ap suat tieu chuan (do C)?', '100', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_014', 'So luong quoc gia tren the gioi (theo LHQ)?', '193', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_015', 'Nam Titanic bi chim?', '1912', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_016', 'Chieu dai cua song Nile (km)?', '6650', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_017', 'Nam con nguoi lan dau dat chan len Mat Trang?', '1969', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_018', 'Toc do am thanh trong khong khi (m/s)?', '343', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_019', 'So luong rang cua nguoi truong thanh (du)?', '32', 3);
INSERT INTO questions (id, text, correct_ans, round_id) VALUES ('q3_020', 'Nam Bitcoin ra doi?', '2009', 3);
COMMIT;