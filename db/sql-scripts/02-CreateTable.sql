CREATE TABLE employees (
first_name varchar(25),
last_name  varchar(25),
department varchar(15),
email  varchar(50)
);

GRANT ALL PRIVILEGES ON company.employees TO 'vluser'@'%';
FLUSH PRIVILEGES;
