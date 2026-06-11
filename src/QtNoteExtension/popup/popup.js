let tempNote = null;
// ========== 核心：宿主名统一为 knot_note_host ==========
const HOST_NAME = "knot_note_host";

document.addEventListener("DOMContentLoaded", async () => {
  const notebookSelect = document.getElementById("notebookSelect");
  const tagInput = document.getElementById("tagInput");
  const saveBtn = document.getElementById("saveBtn");

  // 读取暂存内容
  const storage = await chrome.storage.local.get("tempNote");
  tempNote = storage.tempNote;
  if (!tempNote) {
    alert("未获取到选中内容");
    window.close();
    return;
  }

  // 请求笔记本列表
  try {
    const res = await chrome.runtime.sendNativeMessage(HOST_NAME, {
      action: "getNotebooks"
    });

    if (res && res.notebooks && Array.isArray(res.notebooks)) {
      res.notebooks.forEach(item => {
        const opt = document.createElement("option");
        opt.value = item.id;
        opt.textContent = item.name;
        notebookSelect.appendChild(opt);
      });
    }
  } catch (err) {
    alert("连接 Knot 笔记客户端失败，请先启动软件！");
    window.close();
  }

  // 提交保存
  saveBtn.addEventListener("click", async () => {
    const notebookId = notebookSelect.value;
    const tags = tagInput.value.trim();

    const sendData = {
      action: "saveNote",
      notebookId: notebookId,
      tags: tags,
      ...tempNote
    };

    try {
      await chrome.runtime.sendNativeMessage(HOST_NAME, sendData);
      alert("保存成功！");
      window.close();
    } catch (err) {
      alert("保存失败，请检查客户端状态");
    }
  });
});