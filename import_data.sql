-- Tạo bảng nếu chưa có
CREATE TABLE IF NOT EXISTS questions (
  id TEXT PRIMARY KEY,
  text TEXT,
  opt_a TEXT, opt_b TEXT, opt_c TEXT, opt_d TEXT,
  correct_ans TEXT
);

-- Xóa dữ liệu cũ
DELETE FROM questions;

-- Thêm 30 câu hỏi với đầy đủ 4 đáp án
INSERT INTO questions (id, text, opt_a, opt_b, opt_c, opt_d, correct_ans) VALUES 
('q001', 'CPU la viet tat cua chu gi?', 'Central Processing Unit', 'Central Pretty Unit', 'Core Processing Unit', 'Computer Personal Unit', 'A'),
('q002', 'Thu do cua Viet Nam la gi?', 'TP. Ho Chi Minh', 'Ha Noi', 'Da Nang', 'Hai Phong', 'B'),
('q003', '1 + 1 = ?', '1', '2', '3', '11', 'B'),
('q004', 'RAM la viet tat cua gi?', 'Random Access Memory', 'Read Access Memory', 'Rapid Access Memory', 'Real Access Memory', 'A'),
('q005', 'HTML la viet tat cua gi?', 'HyperText Markup Language', 'HighText Machine Language', 'HyperText Machine Language', 'HighText Markup Language', 'A'),
('q006', 'Python la gi?', 'Ngon ngu lap trinh', 'Loai ran', 'Ca hai deu dung', 'Mot loai virus', 'C'),
('q007', 'HTTP la gi?', 'HyperText Transfer Protocol', 'HighText Transfer Protocol', 'HyperText Transmission Protocol', 'HighText Transmission Protocol', 'A'),
('q008', 'SQL la viet tat cua gi?', 'Structured Query Language', 'Simple Query Language', 'Standard Query Language', 'System Query Language', 'A'),
('q009', 'IP la gi?', 'Internet Protocol', 'Internal Protocol', 'International Protocol', 'Integrated Protocol', 'A'),
('q010', 'DNS la gi?', 'Domain Name System', 'Data Name System', 'Digital Name System', 'Dynamic Name System', 'A'),
('q011', 'URL la viet tat cua gi?', 'Uniform Resource Locator', 'Universal Resource Locator', 'Unified Resource Locator', 'Unique Resource Locator', 'A'),
('q012', 'CSS la gi?', 'Cascading Style Sheets', 'Computer Style Sheets', 'Creative Style Sheets', 'Colorful Style Sheets', 'A'),
('q013', 'JSON la viet tat cua gi?', 'JavaScript Object Notation', 'Java Standard Object Notation', 'JavaScript Online Notation', 'Java Simple Object Notation', 'A'),
('q014', 'API la gi?', 'Application Programming Interface', 'Advanced Programming Interface', 'Automated Programming Interface', 'Application Process Interface', 'A'),
('q015', 'GUI la viet tat cua gi?', 'Graphical User Interface', 'General User Interface', 'Guided User Interface', 'Global User Interface', 'A'),
('q016', 'OS la viet tat cua gi?', 'Operating System', 'Online System', 'Open System', 'Optical System', 'A'),
('q017', 'LAN la gi?', 'Local Area Network', 'Large Area Network', 'Long Area Network', 'Limited Area Network', 'A'),
('q018', 'WAN la gi?', 'Wide Area Network', 'World Area Network', 'Wireless Area Network', 'Web Area Network', 'A'),
('q019', 'USB la viet tat cua gi?', 'Universal Serial Bus', 'Uniform Serial Bus', 'United Serial Bus', 'Universal System Bus', 'A'),
('q020', 'SSD la gi?', 'Solid State Drive', 'Super Speed Drive', 'System State Drive', 'Secure Storage Drive', 'A'),
('q021', 'HDD la gi?', 'Hard Disk Drive', 'High Density Drive', 'Hybrid Disk Drive', 'Heavy Duty Drive', 'A'),
('q022', 'GPU la viet tat cua gi?', 'Graphics Processing Unit', 'General Processing Unit', 'Graphical Performance Unit', 'Game Processing Unit', 'A'),
('q023', 'BIOS la gi?', 'Basic Input/Output System', 'Binary Input/Output System', 'Boot Input/Output System', 'Base Input/Output System', 'A'),
('q024', 'ROM la viet tat cua gi?', 'Read-Only Memory', 'Random-Only Memory', 'Rapid-Only Memory', 'Real-Only Memory', 'A'),
('q025', 'VPN la gi?', 'Virtual Private Network', 'Very Private Network', 'Verified Private Network', 'Visual Private Network', 'A'),
('q026', 'FTP la viet tat cua gi?', 'File Transfer Protocol', 'Fast Transfer Protocol', 'Folder Transfer Protocol', 'Free Transfer Protocol', 'A'),
('q027', 'TCP la gi?', 'Transmission Control Protocol', 'Transfer Control Protocol', 'Transport Control Protocol', 'Terminal Control Protocol', 'A'),
('q028', 'UDP la viet tat cua gi?', 'User Datagram Protocol', 'Universal Datagram Protocol', 'Unified Datagram Protocol', 'Unique Datagram Protocol', 'A'),
('q029', 'SSH la gi?', 'Secure Shell', 'System Shell', 'Safe Shell', 'Standard Shell', 'A'),
('q030', 'HTTPS la viet tat cua gi?', 'HyperText Transfer Protocol Secure', 'HighText Transfer Protocol Secure', 'HyperText Transmission Protocol Secure', 'HyperText Transfer Process Secure', 'A');