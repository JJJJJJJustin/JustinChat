const nodemailer = require('nodemailer');
const configModule = require('./config');

// 创建发送邮件的代理
let transport = nodemailer.createTransport({host:'smtp.163.com', port:465, secure:true, auth:{user:configModule.email_User, pass:configModule.email_Pass}});

// 创建发送邮件的函数
function SendMail(mailOptions)
{
    return new Promise(function(resolve, reject)
    {
        /* 执行器函数的定义 */
        // 使用 sendMail 发送邮件（第一个参数确定发送的地址以及其他信息，第二个参数即回调函数用于监视发送情况，并打印调试信息）
        transport.sendMail(mailOptions, function(error, info)
        {
            if (error) { console.log(error); reject(error); }
            else { console.log('Email.js: 邮件已经成功发送', info.response); resolve(info.response); }
        })
    });
}


module.exports.SendMail = SendMail;