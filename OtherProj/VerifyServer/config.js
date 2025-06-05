const fs = require('fs')

let config = JSON.parse(fs.readFileSync('config.json', 'utf8'));
let email_User = config.email.user;
let email_Pass = config.email.pass;

let mysql_Host = config.mysql.host;
let mysql_Port = config.mysql.port;
let mysql_Password = config.mysql.password;

let mysqlLo_Host = config.mysqlLocalhost.host;
let mysqlLo_Port = config.mysqlLocalhost.port;
let mysqlLo_Password = config.mysqlLocalhost.password;

let redis_Host = config.redis.host;
let redis_Port = config.redis.port;
let redis_Password = config.redis.password;

let redisLo_Host = config.redisLocalhost.host;
let redisLo_Port = config.redisLocalhost.port;
let redisLo_Password = config.redisLocalhost.password;

//let codePrefix = "code_";

module.exports = 
{
    email_User, email_User, email_Pass, 
    mysql_Host, mysql_Port, mysql_Password,
    mysqlLo_Host, mysqlLo_Port, mysqlLo_Password,
    redis_Host, redis_Port, redis_Password,
    redisLo_Host, redisLo_Port, redisLo_Password
    /*, codePrefix*/
};