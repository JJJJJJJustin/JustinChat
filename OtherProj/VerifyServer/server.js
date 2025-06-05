const grpc = require('@grpc/grpc-js')
const {v4: uuidv4} = require('uuid')
const redisModule = require('./redis')
const constModule = require('./const')
const emailModule = require('./email')
const messageProto = require('./proto')

async function GetVerifyCode(call, callback)
{
    console.log(call.request.email, " is handling");
    try
    {
        let queryResult = await redisModule.GetRedis(call.request.email);       // 从数据库中读取验证码，并将其赋予uniqueID
        let uniqueID = queryResult;
        // 若该邮箱的验证码已经超时，不在数据库内存中，则重新发送。并重新入库、重新定时
        if(queryResult === null)
        {
            uniqueID = uuidv4();
            if(uniqueID.length > 4 )
            {
                uniqueID = uniqueID.substring(0, 4);        // 选用验证码的前四位
            }
            console.log(`Value is null, regenerating verification code... (Key:${call.request.email})`);
            // 开始定时，并将新生成的验证码入库
            let bres = await redisModule.SetRedisExpire(call.request.email, uniqueID, 300);
            if(!bres)
            {
                callback(null, { email:call.request.email, error:constModule.Errors.Success });
                return;
            }
        }

        console.log('sending verification code to mail...(code is %s)', uniqueID)
        let textStr = '您的验证码为：' + uniqueID + ', 请在三分钟内完成注册，超时验证码则会失效。';
        
        // 发送邮件（mailOptions 为发送邮件时指定的信息或规范）
        let mailOptions = 
        {
            from: 'your_email',
            to: call.request.email,
            subject: '验证码',
            text: textStr
        }
        let sendRes = await emailModule.SendMail(mailOptions);
        // console.log('Send response is ', sendRes);

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