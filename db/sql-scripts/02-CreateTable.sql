CREATE SCHEMA company;

CREATE TABLE company.employees (
first_name varchar(25),
last_name  varchar(25),
department varchar(15),
email  varchar(50)
);

GRANT ALL PRIVILEGES ON company.employees TO 'vluser'@'%';
FLUSH PRIVILEGES;

CREATE TABLE virtual_lab.models (
  `id` INT NOT NULL,
  `name` VARCHAR(45) NOT NULL,
  `version` INT NOT NULL,
  PRIMARY KEY (`id`));


GRANT ALL PRIVILEGES ON virtual_lab.models TO 'vluser'@'%';
FLUSH PRIVILEGES;