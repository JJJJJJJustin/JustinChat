const fs = require('fs')

let config = JSON.parse(fs.readFileSync('config.json', 'utf8'));
let emailUser = config.email.user;
let emailPass = config.email.pass;
let mysqlHost = config.mysql.host;
let mysqlPort = config.mysql.port;
let mysqlPassword = config.mysql.password;
let redisHost = config.redis.host;
let redisPort = config.redis.port;
let redisPassword = config.redis.password;
//let codePrefix = "code_";

module.exports = {emailUser, emailPass, mysqlHost, mysqlPort, mysqlPassword, redisHost, redisPort, redisPassword/*, codePrefix*/};