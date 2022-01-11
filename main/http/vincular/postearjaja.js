const gel = (e) => document.getElementById(e);



gel("btnVincular").addEventListener(
    "click",
    (e) => {
        // si let no funca, pone var
        let codigoID = gel("codigoID").value;

        //El connect.json tambien se declara en c, ponele el nombre que mas te guste
        let test = fetch("/vincular.json", {
            method: "POST",
            //El X-Codigo-Id lo tenes que declarar en C
            headers: {
                "Content-Type": "application/json",
                "X-Codigo-Id": codigoID
            },

            body: { timestamp: Date.now() },
        });
        let msg = document.getElementsByClassName('message');
        msg.style.display = 'block';
        msg.textContent  = test['message'];
        
    },
    false
);
