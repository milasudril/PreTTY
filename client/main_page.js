window.onbeforeunload = function()
{
	var req = new XMLHttpRequest();
	req.open("GET", "/!shutdown?api_key=$api_key");
	req.setRequestHeader("Content-Type", "application/json");
	req.send(JSON.stringify({}));
}

function maximize_output()
{
	document.getElementById("source_editor").style.width = "0%";
	document.getElementById("output_frame").style.width = "100%";
	document.getElementById("output_frame").style.width = "max-content";
	document.getElementById("output_frame").focus();
}

function maximize_editor()
{
	document.getElementById("source_editor").style.width = "100%";
	document.getElementById("output_frame").style.width = "0%";
	document.getElementById("source_editor").focus();
}

function source_editor_key_handler(e)
{
	if(e.key === "Tab" && !e.altKey && !e.ctrlKey)
	{
		e.preventDefault();
		const textArea = e.currentTarget;
		if(!e.shiftKey)
		{ textArea.setRangeText("\t", textArea.selectionStart, textArea.selectionEnd, "end"); }
	}
	else
	if(e.key === "Enter" && e.ctrlKey)
	{
		document.getElementById("build_and_run").click();
	}
	else
	if(e.key === "ArrowRight" && e.ctrlKey)
	{
		e.preventDefault();
		document.getElementById("build_and_run").click();
		maximize_output();

	}
	else
	if(e.key === "ArrowLeft" && e.ctrlKey)
	{
		e.preventDefault();
		maximize_editor();
	}
	else
	if(e.key === "Escape")
	{
		document.getElementById("filename").focus()
	}
}

function document_on_key_down(e)
{
	if(e.key === "ArrowRight" && e.ctrlKey)
	{
		e.preventDefault();
		document.getElementById("build_and_run").click();
		maximize_output();

	}
	else
	if(e.key === "ArrowLeft" && e.ctrlKey)
	{
		e.preventDefault();
		maximize_editor();
	}
	else
	if(e.key === "1" && e.ctrlKey)
	{
		e.preventDefault();
		document.getElementById("load").click();
	}
	else
	if(e.key === "2" && e.ctrlKey)
	{
		e.preventDefault();
		document.getElementById("save").click();
	}
}

function set_panel_size(e)
{
  document.getElementById("source_editor").style.width = "50%";
}