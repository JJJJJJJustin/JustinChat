#include "JCpch.h"
#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MySqlMgr.h"

// Constructor is private
LogicSystem::LogicSystem()
{
	// ����洢���ڿͻ��� GET ����ļ򵥴���������һЩ�򵥵��ַ���
	// "/get_test" ��ʾ�ͻ������������ַ�� http://localhost:8080/get_test 
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Recevie get_test request :-) \n";

			int i = 0;
			for (auto& elem : connection->m_GetParams) 
			{
				i++;
				beast::ostream(connection->m_Response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->m_Response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		}
	);

	// "/" ��ʾ�ͻ������������ַ�� http://localhost:8080/
	RegGet("/", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Haha Ha HHHa \n";

			int i = 0;
			for (auto& elem : connection->m_GetParams) 
			{
				i++;
				beast::ostream(connection->m_Response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->m_Response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		}
	);

	// ����洢���ڿͻ��� POST ����ļ򵥴�����
	RegPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection) 
		{
			Json::Value reqRoot, rspRoot;
			Json::Reader reader;
						
			auto reqData = boost::beast::buffers_to_string(connection->m_Request.body().data());
			JC_CORE_INFO("Receive body is \n{}\n", reqData);
			bool success = reader.parse(reqData, reqRoot);
			if(!success)
			{
				JC_CORE_ERROR("Failed to parse JSON data!\n");
				rspRoot["error"] = ErrorCodes::Error_Json;
				boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();

				return true;
			}

			std::string email = reqRoot["email"].asString();
			message::GetVerifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);

			rspRoot["error"] = rsp.error();
			rspRoot["email"] = email;
			JC_CORE_TRACE("Handling email: {}...\n", email);

			boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();
			return true;
		}
	);

	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) 
		{
			Json::Value reqRoot, rspRoot;
			Json::Reader reader;

			auto reqData = boost::beast::buffers_to_string(connection->m_Request.body().data());
			JC_CORE_INFO("Receive body id \n {}", reqData);
			bool success = reader.parse(reqData, reqRoot);
			if (!success)
			{
				JC_CORE_ERROR("Failed to parse JSON data!");
				rspRoot["error"] = ErrorCodes::Error_Json;
				boost::beast::ostream(connection->m_Request.body()) << rspRoot.toStyledString();

				return true;		// lambda ���ʽ�е� return true ��ʾ�����ñ��ʽ
			}

			auto user = reqRoot["user"].toStyledString();
			auto email = reqRoot["email"].toStyledString();
			auto password = reqRoot["password"].toStyledString();
			auto confirm = reqRoot["confirm"].toStyledString();
			auto verifycode = reqRoot["verifycode"].toStyledString();

			if(strcmp(password.c_str(), confirm.c_str()))
			{
				JC_CORE_ERROR("Password and confirm are not equal");
				rspRoot["error"] = ErrorCodes::Error_Password_Incorrect;
				boost::beast::ostream(connection->m_Request.body()) << rspRoot.toStyledString();

				return true;
			}

			// �����֤���Ƿ����
			std::string verifyCode;
			bool getResult = RedisMgr::GetInstance()->Get(reqRoot["email"].asString(), verifyCode);
			if(!getResult)
			{
				JC_CORE_ERROR("Can't find {} in redis database!", reqRoot["email"].toStyledString());
				rspRoot["error"] = ErrorCodes::Error_Verify_Expired;
				boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();

				return true;
			}
			// �����֤���� redis ���ݿ��е������Ƿ�ƥ��
			if (reqRoot["verifycode"].asString() != verifyCode)
			{
				JC_CORE_ERROR("Verify code error!");
				JC_CORE_ERROR("reqRoot email asString() == {}", reqRoot["email"].asString());
				JC_CORE_ERROR("verifyCode == {}", verifyCode);

				rspRoot["error"] = ErrorCodes::Error_Verify_Code;
				boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();

				return true;
			}

			// ʹ�� mysql ��ѯ�û��Ƿ����
			int uid = MySqlMgr::GetInstance()->RegUser(user, email, password);

			if (uid == 0 || uid == -1) 
			{
				JC_CORE_ERROR("User or email is not exist!");

				rspRoot["error"] = ErrorCodes::Error_User_Exist;
				boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();

				return true;
			}

			// �� qt ���ͻذ�
			rspRoot["error"] = 0;
			rspRoot["uid"] = uid;
			rspRoot["user"] = reqRoot["user"].toStyledString();
			rspRoot["email"] = reqRoot["email"].toStyledString();
			rspRoot["password"] = reqRoot["password"].toStyledString();
			rspRoot["confirm"] = reqRoot["confirm"].toStyledString();
			rspRoot["verifycode"] = reqRoot["verifycode"].toStyledString();

			boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();
			return true;
		}
	);


}

LogicSystem::~LogicSystem()
{
}

void LogicSystem::RegGet(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func)
{
	m_GetHandlers.insert(std::make_pair(url, func));
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if(m_GetHandlers.find(path) == m_GetHandlers.end())
	{
		JC_CORE_ERROR("Can't find '{0}' in GetHandlers, FILE:{1}", path, __FILE__)
		return false;
	}

	m_GetHandlers[path](connection);
	return true;
}

void LogicSystem::RegPost(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func)
{
	m_PostHandlers.insert(std::make_pair(url, func));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if (m_PostHandlers.find(path) == m_PostHandlers.end())
	{
		JC_CORE_ERROR("Can't find '{0}' in GetHandlers, FILE:{1}", path, __FILE__)
			return false;
	}

	m_PostHandlers[path](connection);
	return true;
}

