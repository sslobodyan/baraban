
var CreateTablePots = function() {
	var tbody = $('#table_pots');
	tbody.html("");
	if (tbody) {
        for (var i = 0; i < 3; i++) {
			var s = '<tr>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td>'+i+'</td>';
			s += '<td><button>Load</button><button>Save</button></td>';
			s += '</tr>';
            tbody.append(s);
        };
	};
};

var setupPots = function() {
	var but = document.getElementById('LoadAll_Pots');
	but.addEventListener('click', CreateTablePots);
}

