# دليل التثبيت (مبني على الأدلة)

**الإصدار:** 0.4.1-phase4 (`include/elx_config.h`).
**المتحقق محلياً:** Linux x64 (بناء + `make test`). بقية الأنظمة: ملفات مصدر/بناء موجودة لكن **غير مُتحقق منها** هنا.

## تصنيف المنصات
| المنصة | الحالة | ملاحظات |
|---|---|---|
| Linux x64 | مُتحقق | نجح البناء والاختبار محلياً |
| Debian/Ubuntu/Kali | متوافق للبناء | مقترح: `build-essential` (تحقق على نظامك) |
| Arch | متوافق للبناء | مقترح: `base-devel` |
| Fedora | متوافق للبناء | مقترح: أدوات التطوير |
| macOS | متوافق للبناء | يحتاج Xcode CLT؛ فرع Darwin في Makefile؛ غير مُختبر |
| Windows 10/11 (MinGW/MSYS2) | متوافق للبناء | فرع `Windows_NT` + `-lws2_32`؛ غير مُختبر |
| Windows (MSVC) | تجريبي | Makefile موجه لـ gcc؛ غير مُتحقق |
| ARM64 | غير مُتحقق | تعريفات موجودة؛ لا دليل بناء/اختبار |

## Linux (المسار المُتحقق)
```bash
cd elx-sdk
make clean && make all
./build/elx version && ./build/elx doctor && make test
```

## macOS / Windows
راجع التعليمات الإنجليزية التفصيلية في [../INSTALLATION.md](../INSTALLATION.md) مع اعتبار الحالة “غير مُتحقق” حتى يثبت البناء والاختبار على جهازك.
