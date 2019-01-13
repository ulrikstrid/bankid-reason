open Lwt_result.Infix;

let connection_error_to_string =
  fun
  | `Exn(exn) => Printexc.to_string(exn)
  | `Invalid_response_body_length(_res) => "Invalid Response Body Length"
  | `Malformed_response(str) => "Malformed Response: " ++ str;

let error_to_string =
  fun
  | `Connection_error(error) => connection_error_to_string(error)
  | `Reading_error => "Something went wrong when reading the body";

let sendReqWithConfig = (config, req): Lwt.t(result(string, 'a)) =>
  Httpkit_lwt.Client.(Https.send(~config, req) >>= Response.body);
