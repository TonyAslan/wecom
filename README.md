# Wecom
基于开源界面实现的一个比较完整的企业微信小项目
分为客户端服务端、数据库，基本框架如下：
<img width="463" height="375" alt="image" src="https://github.com/user-attachments/assets/85eee24a-af68-4386-83e2-e3b128b85352" />

# 客户端
客户端界面参考开源项目（https://gitee.com/hudejie/wecom-copy）

# 服务端
## 登录：
### 客户端
客户端发送登录请求给服务端，服务端处理请求并向客户端发送响应。用户账号密码信息存储于 MySQL 数据库。
登录对话框（CLoginDlg）内使用 TCP 连接到服务端。
用户通过账号输入框及密码输入框输入账号密码，并点击登录按钮。
登录按钮信号槽内获取用户输入的账号密码，通过登录请求格式打包请求，之后通过 TCP 将请求发往服务端。登录请求格式如下：
{ 
"type": "0", 
"login": {
 "account": "zhangsan", 
"password": "encrypted_password" 
}
}
### 服务端
服务端通过 QSqlDatabase 连接到 MySQL，并使用 QTcpServer 处理 TCP 连接。
服务端接收客户端消息，解析数据包，获取请求数据。
假设请求为登录请求，服务端通过数据库验证“账号“和“密码”，如果验证成功还需要获取用户相关信息（用户信息及好友信息），
之后将验证结果和用户相关信息打包并向客户端发送登录响应。登录响应格式如下：
{
"type": "0", 
"data": { 
"userId": 1001, 
"userName": "张三", 
"userPart": "研发部", 
"userEmail": "zhangsan@example.com", 
"userImg": "img/1001.jpg", 
"list": [ 
{"id": 1002, "name": "李四"}, 
{"id": 1003, "name": "王五"}, 
{"id": 1004, "name": "赵六"}, 
... // 其他好友（精简信息）
 ],
"friendCount": 235
 }, 
"status": 1000,
 "desc": "OK" 
}
数据使用 Json 对象封装。"type"表示消息类型，"0"表示登录消息；"data" 对应一个 Json 对象，该对象内封装用户相关信息；"status"表示状态码；“desc”表示验证是否成功。
考虑到好友太多，所以首先仅加载精简好友信息，而后分段加载好友详情。
## 查好友详情
### 客户端
客户端请求好友详情：
{ 
"type": "2", 
"friendIds": [1002, 1003, 1007] // 需要详情的好友ID 
}
### 服务端
服务端好友详情相应：
{ 
"type": "2", 
"data": { 
"friendDetails": [
 { 
"id": 1002, 
"name": "李四", 
"part": "研发部", 
"email": "lisi@example.com", 
"img": "img/1002.jpg"
 }, 
{ 
"id": 1003, 
"name": "王五", 
"part": "市场部", 
"email": "wangwu@example.com", 
"img": "img/1003.jpg" 
}, 
{ 
"id": 1007, 
"name": "郑七", 
"part": "人力资源", 
"email": "zhengqi@example.com",
"img": "img/1007.jpg" 
} 
] 
}, 
"status": 1000, 
"desc": "OK"
 }

## 文本聊天
用户与用户之间进行聊天通信，服务端做消息转发。基本框架如下：








<img width="508" height="406" alt="image" src="https://github.com/user-attachments/assets/ef33cec3-da61-4835-bd59-a7562d5e0296" />



### 客户端
当用户点击发送按钮，客户端获取用户输入，并且使用 Tcp 将打包好的消息发往服务端，请求服务端做消息转发。
消息转发请求格式如下：
{ "type" : "1", "data" :{ "send" : "10001", "recv" : "10002", "what" : "msg", "msg" :
[
[ {"txt" : "你好"}, {"txt" : "世界"} ],
...
]
}
}
"type"表示消息类型，"1"表示转发消息；"data" 对应一个 Json 对象，该对象内封装聊天用户信息以及消息内容，对象属性含义：
"send" 消息发送者 id
"recv" 消息接收者 id
"what" 消息内容类型，例如："msg"表示消息内容是文本
"msg" 该属性对应消息内容类型，可获取消息内容

### 服务端
服务端通过 TCP 接收到客户端发送的数据，解析数据包，获取请求类型。
假设请求为转发请求，服务端直接通过 UDP 广播刚刚接收到的数据，使所有客户端都收到该数据。
客户端接收到数据之后，再根据"data"对象内的"send"和"recv"属性值来决定是否接收处理该数据
（即"send"属性值不等于登陆者 id 并且"recv"属性值等于登陆者 id）。

# 其他
计划扩展语音、视频通话，暂时未实现

# 数据库
简单实现数据库，表如下：







好友关系表：
<img width="264" height="253" alt="image" src="https://github.com/user-attachments/assets/0bdb6209-a9cc-4e43-b7c3-d70ac00448aa" />
用户登录表：
<img width="496" height="373" alt="image" src="https://github.com/user-attachments/assets/5e6b3d1b-ac38-4fde-8c24-5d39b7e537a6" />
用户详情表：
<img width="869" height="387" alt="image" src="https://github.com/user-attachments/assets/62f62755-8aa2-4912-9da2-1752ef4af127" />


