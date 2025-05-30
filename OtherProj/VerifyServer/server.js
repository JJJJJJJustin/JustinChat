const grpc = require('@grpc/grpc-js')
const {v4: uuidv4} = require('uuid')
const constModule = require('./const')
const emailModule = require('./email')
const messageProto = require('./proto')

async function GetVerifyCode(call, callback)
{
    console.log(call.request.email, " is handling");
    try
    {
        let uniqueID = uuidv4();
        console.log('sending verification code to mail...(code is %s)', uniqueID)
        let textStr = '您的验证码为：' + uniqueID + ', 请在规定时间内完成注册，超时验证码则会失效。';
        
        // 发送邮件（mailOptions 为发送邮件时指定的信息或规范）
        let mailOptions = 
        {
            from: 'your_email',
            to: call.request.email,
            subject: '验证码',
            text: textStr
        }
        let sendRes = await emailModule.SendMail(mailOptions);
        console.log('Send response is ', sendRes);

        // 如果 try 中的代码正常执行，则会运行 callback 函数
        // 该函数会将错误信息设置为null，并且设置传递结果（此处包括 email 地址和状态码 error ）
        callback(null, { email: call.request.email, error: constModule.Errors.Success });
    }
    catch(error)
    {
        console.log("Catch the error is:", error);

        callback(null, { email: call.request.email, error: constModule.Errors.Exception} );
    }
}

function main()
{
    var server = new grpc.Server();
    // 添加服务的函数定义（处理逻辑）
    server.addService(messageProto.VerifyService.service, { GetVerifyCode, GetVerifyCode });
    // 将 grpc 服务绑定在 50051 端口，并指定配置（Insercure 类型连接），然后在回调中启动监听和日志）
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), ()=>{server.start(); console.log('Grpc server started!')});
}

main();