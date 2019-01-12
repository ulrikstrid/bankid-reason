let req = pno =>
  Uri.of_string("https://appapi2.test.bankid.com/rp/v5/auth")
  |> Httpkit.Client.Request.create(
       ~body=
         {|{"personalNumber":"|} ++ pno ++ {|","endUserIp":"94.191.144.73"}|},
       ~headers=[("Content-Type", "application/json")],
       `POST,
     );

let auth = (config, pno) => req(pno) |> Util.sendReqWithConfig(config);
